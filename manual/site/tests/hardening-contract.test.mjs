import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtempSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join, resolve } from 'node:path';
import test from 'node:test';
import { fileURLToPath } from 'node:url';
import { load as loadYaml } from 'js-yaml';

const site = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const repository = resolve(site, '../..');
const readRepositoryFile = (path) => readFileSync(resolve(repository, path), 'utf8');

test('manual workflow separates cancellable validation from non-cancellable deployment', () => {
	const source = readRepositoryFile('.github/workflows/manual-pages.yml');
	const workflow = loadYaml(source);
	const validation = workflow.jobs['validate-and-build'];
	const deployment = workflow.jobs.deploy;

	assert.equal(workflow.concurrency, undefined, 'workflow-level cancellation could interrupt deployment');
	assert.equal(validation.concurrency['cancel-in-progress'], true);
	assert.match(validation.concurrency.group, /manual-validation/);
	assert.equal(deployment.concurrency.group, 'github-pages');
	assert.equal(deployment.concurrency['cancel-in-progress'], false);
	for (const trigger of ['pull_request', 'push']) {
		for (const path of ['manual/**', 'README.md', 'CONTRIBUTING.md', 'AGENTS.md', 'LICENSE.md', 'docs/**']) {
			assert.ok(workflow.on[trigger].paths.includes(path), `${trigger} must include ${path}`);
		}
	}

	const step = (name) => validation.steps.find((candidate) => candidate.name === name);
	const pushCheck = step('Run complete manual check for a push');
	const zeroSha = '0'.repeat(40);
	assert.ok(pushCheck, 'push validation must cover every push shape');
	assert.equal(pushCheck.if, "github.event_name == 'push'");
	assert.match(pushCheck.run, new RegExp(zeroSha), 'branch creation must be recognized');
	assert.match(pushCheck.run, /git cat-file -e/, 'a force-push may name a revision the history no longer contains');
	assert.match(pushCheck.run, /--base-ref/);
	assert.match(pushCheck.run, /git merge-base HEAD origin\/main/, 'an unavailable base must fall back to where the branch left main');

	const install = step('Install production site dependencies');
	const build = step('Build publish artifact from production dependencies');
	const verify = step('Verify publish artifact');
	assert.ok(install && build && verify, 'production artifact steps are required');
	assert.equal(install.if, undefined, 'pull requests must verify production dependencies too');
	assert.equal(build.if, undefined);
	assert.equal(verify.if, undefined);
	assert.match(install.run, /npm ci --omit=dev/);
	assert.match(build.run, /npm run build/);
	for (const command of ['check:render', 'check:search', 'check:links']) {
		assert.match(verify.run, new RegExp(`npm run ${command}`));
	}
	assert.ok(validation.steps.indexOf(verify) < validation.steps.findIndex((candidate) => candidate.name === 'Upload Pages artifact'));
	assert.doesNotMatch(source, /\bPUBLIC_(?:DOCS_ORIGIN|REPOSITORY_URL|GIT_SHA)\b/);

	// The gate builds with the fixtures, so these steps are where a fixture page
	// leaking into a publishable artifact would be caught.
	assert.doesNotMatch(build.run, /MANUAL_TEST_FIXTURES/);
	assert.doesNotMatch(verify.run, /MANUAL_TEST_FIXTURES/);
});

test('the gate builds the site once and keeps the checks that need no build out of it', () => {
	const engine = readRepositoryFile('manual/tools/manage_engine.py');
	const checks = engine.slice(engine.indexOf('def run_site_checks'));
	assert.equal(checks.match(/"run", "build"/g)?.length, 1, 'the gate must build once');
	assert.match(checks, /MANUAL_TEST_FIXTURES"\] = "1"/);
	assert.match(checks, /unittest", "discover"/, 'the Python tests must stay in the gate');
	assert.match(checks, /glob\("test_\*\.py"\)/, 'every Python test module must be discovered');
	for (const command of ['check:render', 'check:search', 'check:links', 'check:docs']) {
		assert.match(checks, new RegExp(command.replace(':', ':')), `${command} must stay in the gate`);
	}

	const scripts = JSON.parse(readRepositoryFile('manual/site/package.json')).scripts;
	assert.equal(scripts['check:links'], 'node scripts/check-links.mjs',
		'check:docs reads the repository rather than dist, so it runs on its own');
});

test('workflow consumes native toolchain authorities instead of version literals', () => {
	const nodeVersion = readRepositoryFile('manual/site/.nvmrc').trim();
	const pythonVersion = readRepositoryFile('manual/tools/.python-version').trim();
	const packageJson = JSON.parse(readRepositoryFile('manual/site/package.json'));
	const npmVersion = packageJson.packageManager.match(/^npm@(\d+\.\d+\.\d+)$/)?.[1];
	const source = readRepositoryFile('.github/workflows/manual-pages.yml');
	const workflow = loadYaml(source);
	const steps = workflow.jobs['validate-and-build'].steps;

	assert.match(nodeVersion, /^\d+\.\d+\.\d+$/);
	assert.equal(packageJson.engines.node, nodeVersion);
	assert.match(pythonVersion, /^\d+\.\d+\.\d+$/);
	assert.ok(npmVersion, 'packageManager must pin a complete npm version');
	assert.equal(steps.find((step) => step.name === 'Set up Python').with['python-version-file'], 'manual/tools/.python-version');
	assert.equal(steps.find((step) => step.name === 'Set up Node').with['node-version-file'], 'manual/site/.nvmrc');
	assert.doesNotMatch(source, /^\s*python-version:\s*["']?\d/m);
	assert.doesNotMatch(source, /^\s*node-version:\s*["']?\d/m);
	assert.doesNotMatch(source, /npm install --global npm@\d/);
});

test('build-time packages are production dependencies and tooling remains development-only', () => {
	const packageJson = JSON.parse(readRepositoryFile('manual/site/package.json'));
	for (const dependency of ['@astrojs/markdown-remark', 'ajv']) {
		assert.ok(packageJson.dependencies[dependency], `${dependency} must be available to production builds`);
		assert.equal(packageJson.devDependencies[dependency], undefined);
	}
	assert.equal(packageJson.dependencies.sharp, undefined, 'sharp is not used directly');
	assert.equal(packageJson.devDependencies.sharp, undefined);
	for (const dependency of ['@astrojs/check', 'typescript']) {
		assert.ok(packageJson.devDependencies[dependency], `${dependency} is development-only tooling`);
	}
});

test('fixture checks require the synthetic page in fixture mode and reject it in production', () => {
	const render = readRepositoryFile('manual/site/scripts/check-render.mjs');
	const search = readRepositoryFile('manual/site/scripts/check-search.mjs');
	for (const source of [render, search]) {
		assert.match(source, /process\.env\.MANUAL_TEST_FIXTURES === '1'/);
		assert.match(source, /existsSync\(fixturePage\) !== fixturesEnabled/);
		assert.match(source, /Fixture build is missing keys\/oldexample\/index\.html/);
		assert.match(source, /Production build unexpectedly contains keys\/oldexample\/index\.html/);
		assert.doesNotMatch(source, /if\s*\(\s*existsSync\(resolve\(['"]dist\/keys\/oldexample/);
	}
	assert.match(search, /expectRoute\('OldExample'/);
	assert.match(search, /expectNoRoute\('OldExample'/);
});

test('repository Markdown checker rejects missing targets and fragments', () => {
	const fixture = mkdtempSync(join(tmpdir(), 'opents-doc-links-'));
	const checker = resolve(site, 'scripts/check-doc-links.mjs');
	const run = () => spawnSync(process.execPath, [checker, '--root', fixture], { encoding: 'utf8' });
	try {
		writeFileSync(resolve(fixture, 'README.md'), '# Fixture\n\n[Details](details.md#target-heading)\n');
		writeFileSync(resolve(fixture, 'details.md'), '# Target heading\n');
		let result = run();
		assert.equal(result.status, 0, result.stderr);

		writeFileSync(resolve(fixture, 'README.md'), '# Fixture\n\n[Missing](missing.md)\n');
		result = run();
		assert.equal(result.status, 1);
		assert.match(result.stderr, /missing target missing\.md/);

		writeFileSync(resolve(fixture, 'README.md'), '# Fixture\n\n[Details](details.md#missing-heading)\n');
		result = run();
		assert.equal(result.status, 1);
		assert.match(result.stderr, /missing fragment #missing-heading/);
	} finally {
		rmSync(fixture, { recursive: true, force: true });
	}
});
