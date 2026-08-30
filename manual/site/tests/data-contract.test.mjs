import assert from 'node:assert/strict';
import { readFileSync, readdirSync } from 'node:fs';
import test from 'node:test';
import { load } from 'js-yaml';
import { compare } from 'semver';
import {
	applicabilitySummary,
	keySlug,
	OTHER_INI_FILES,
	referenceFileLabel,
	referenceGroups,
	scenarioGroups,
	slugify,
} from '../src/lib/reference.mjs';
import {
	isImageSection,
	requireSectionSelector,
	sectionSelectorKey,
	sectionSelectorLabel,
	sectionSelectorSlug,
} from '../src/lib/section-selector.mjs';
import {
	formatWhenOmitted,
	summarizeWhenOmitted,
	whenOmittedForScope,
} from '../src/lib/omission.mjs';
import { fixturesEnabled, withFixtures } from '../src/lib/fixtures.mjs';

const yaml = (name) => load(readFileSync(new URL('../../data/' + name, import.meta.url), 'utf8'));
const keys = yaml('ini-keys.yaml');
const scripting = yaml('scripting.yaml');
const tombstoneFixture = load(readFileSync(new URL('./fixtures/tombstone.yaml', import.meta.url), 'utf8'));
const releases = yaml('releases.yaml').releases;
const scriptingAliases = yaml('scripting-route-aliases.yaml');

test('current generated datasets use structural contracts instead of milestone counts', () => {
	assert.ok(Object.keys(keys).length > 0);
	for (const rows of [
		scripting.trigger_actions,
		scripting.trigger_events,
		scripting.team_missions,
	]) {
		assert.ok(rows.length > 0);
		assert.deepEqual(rows.map((row) => row.index), rows.map((_, index) => index));
		assert.equal(new Set(rows.map((row) => row.id)).size, rows.length);
	}
	assert.equal(keys.Inaccuate, undefined);
});

test('map-seed and theater-control references cover every extracted scope', () => {
	const scopes = Object.entries(keys).flatMap(([key, entry]) =>
		entry.scopes.map((scope) => ({ key, scope })));
	const mapSeeds = scopes.filter(({ scope }) => scope.file === 'map seed file');
	const theaterControls = scopes.filter(({ scope }) => scope.file === 'theater control file');
	const theaterGeneral = theaterControls.filter(({ scope }) =>
		sectionSelectorKey(scope.section) === 'literal:General');
	const theaterTileSets = theaterControls.filter(({ scope }) =>
		sectionSelectorKey(scope.section) === 'identifier:tile-set');

	assert.ok(mapSeeds.length > 0);
	assert.ok(mapSeeds.every(({ scope }) => sectionSelectorKey(scope.section) === 'literal:RandomMap'));
	assert.ok(theaterControls.length > 0);
	/* Coverage is the claim, so the two selector families have to account for every
	   theater-control scope between them. A third family arriving unannounced is the
	   regression a total would only have reported as a different number. */
	assert.ok(theaterGeneral.length > 0);
	assert.ok(theaterTileSets.length > 0);
	assert.equal(theaterGeneral.length + theaterTileSets.length, theaterControls.length);
});
test('canonical key routes are unique', () => {
	const routes = Object.entries(keys).map(([name, entry]) => keySlug(name, entry));
	assert.equal(new Set(routes).size, routes.length);
});

test('suffixed scope routes follow recorded content rather than extraction order', () => {
	// `client-settings-2` and its kin are handed out by position, so a reader
	// discovered earlier in the tree could silently swap two published scope
	// routes. The catalog orders such scopes by their own recorded fields; if
	// this ever stops holding, the swap is a route change and must be deliberate.
	const routeBase = (scope) => slugify(scope.applies_to?.[0] ?? 'global');
	const signature = (scope) => [
		scope.level ?? '',
		scope.file ?? '',
		scope.value_type ?? '',
		JSON.stringify(scope.section ?? null),
		JSON.stringify([...(scope.applies_to ?? [])].sort()),
		scope.note ?? '',
		scope.precedence ?? '',
	];
	let suffixed = 0;
	for (const [name, entry] of Object.entries(keys)) {
		const seen = new Map();
		for (const scope of entry.scopes) {
			const base = routeBase(scope);
			const current = signature(scope);
			const previous = seen.get(base);
			if (previous) {
				suffixed += 1;
				const order = previous.findIndex((value, index) => value !== current[index]);
				assert.ok(
					order === -1 || previous[order] < current[order],
					`${name}: scopes sharing the route id "${base}" are out of recorded order`,
				);
			}
			seen.set(base, current);
		}
	}
	assert.ok(suffixed > 0, 'no key exercises a suffixed scope route');
});

test('release registry and legacy scripting aliases use stable structured identities', () => {
	assert.equal(releases.filter((release) => release.status === 'development').length, 1);
	assert.ok(compare('1.10.0', '1.9.0') > 0);
	assert.ok(compare('1.0.0', '1.0.0-rc.1') > 0);
	assert.equal(scriptingAliases.actions['41'], 'TACTION_PLAY_ANIM');
	assert.equal(scriptingAliases.events['34'], 'TEVENT_NEAR_WAYPOINT');
	assert.equal(scriptingAliases.missions['6'], 'TMISSION_LOOP');
	assert.equal(new Set(Object.values(scriptingAliases.actions)).size,
		Object.keys(scriptingAliases.actions).length);
});

test('case-colliding names receive distinct canonical routes and collisions are detectable', () => {
	assert.equal(keySlug('DropPod', keys.DropPod), 'droppod-global-rules');
	assert.equal(keySlug('Droppod', keys.Droppod), 'droppod-teamtype');
	const invalid = [
		keySlug('SyntheticKey', { scopes: [{}] }),
		keySlug('synthetickey', { scopes: [{}] }),
	];
	assert.notEqual(new Set(invalid).size, invalid.length);
});

test('every generated section and secondary read uses the exact selector contract', () => {
	const selectors = [];
	for (const [key, entry] of Object.entries(keys)) {
		for (const [index, scope] of entry.scopes.entries()) {
			selectors.push(requireSectionSelector(scope.section, `${key} scope ${index + 1}`));
			for (const [readIndex, read] of (scope.read_from ?? []).entries()) {
				selectors.push(requireSectionSelector(read.section, `${key} read ${readIndex + 1}`));
			}
		}
	}
	assert.ok(selectors.some((selector) => selector.kind === 'literal'));
	assert.ok(selectors.some((selector) => selector.kind === 'identifier'));
	assert.ok(selectors.some((selector) => selector.kind === 'image'));
	// An Image= read either keeps the object's own entry as its fallback or has
	// no fallback at all, and both shapes are closed.
	assert.ok(selectors.some((selector) => selector.kind === 'image' && selector.fallback === 'object-type'));
	assert.ok(selectors.some((selector) => selector.kind === 'image' && selector.fallback === undefined));
	assert.ok(selectors.every((selector) => {
		const expected = selector.kind === 'literal'
			? ['kind', 'name']
			: selector.kind === 'identifier'
				? ['kind', 'source']
				: selector.fallback === undefined
					? ['kind']
					: ['fallback', 'kind'];
		return JSON.stringify(Object.keys(selector).sort()) === JSON.stringify(expected.sort());
	}));
	assert.throws(
		() => requireSectionSelector("the object's own entry", 'legacy'),
		/section must be a selector object/,
	);
	assert.throws(
		() => requireSectionSelector({ kind: 'literal', name: '[General]' }, 'literal'),
		/invalid section selector/,
	);
});

test('selector labels are literal INI forms and selector slugs preserve published routes', () => {
	assert.equal(sectionSelectorLabel({ kind: 'literal', name: 'General' }), '[General]');
	assert.equal(sectionSelectorLabel({ kind: 'identifier', source: 'object-type' }), '[<ObjectType ID>]');
	assert.equal(sectionSelectorLabel({ kind: 'image', fallback: 'object-type' }), '[<Image ID>]');
	assert.equal(sectionSelectorKey({ kind: 'identifier', source: 'house' }), 'identifier:house');
	assert.equal(sectionSelectorSlug({ kind: 'literal', name: 'LEVITATION' }), 'levitation-controls');
	assert.equal(sectionSelectorSlug({ kind: 'identifier', source: 'house' }), 'the-house-s-own-entry');
});

test('Art uses effective type views and keeps image selectors as row data', () => {
	const building = referenceGroups(keys, 'art').find((group) => group.slug === 'buildingtype');
	assert.ok(building);
	assert.ok(building.rows.length > 20);
	assert.ok(building.rows.some((row) => row.viaImage));
	assert.ok(building.rows.some((row) => !row.viaImage));
	assert.equal(isImageSection({ kind: 'image', fallback: 'object-type' }), true);
	assert.equal(isImageSection({ kind: 'identifier', source: 'object-type' }), false);
});

test('long inherited applicability is condensed without changing exact coverage', () => {
	const abstract = referenceGroups(keys, 'rules').find((group) => group.slug === 'abstracttype');
	assert.ok(abstract);
	assert.equal(abstract.subtitle, undefined);
	const exact = [...new Set(abstract.rows.flatMap((row) => row.scope.applies_to))];
	assert.ok(exact.length > 4);
	assert.equal(applicabilitySummary(exact, 'AbstractTypeClass'), 'AbstractType and derived types');
	assert.equal(applicabilitySummary(exact.slice(0, 4), 'AbstractTypeClass'), exact.slice(0, 4).join(', '));
});

test('authored enum domains are complete, uniquely bound, and include representative values', () => {
	const directory = new URL('../../content/enums/', import.meta.url);
	const records = readdirSync(directory)
		.filter((name) => name.endsWith('.md'))
		.map((name) => {
			const source = readFileSync(new URL(name, directory), 'utf8');
			const match = source.match(/^---\r?\n([\s\S]*?)\r?\n---/);
			assert.ok(match, name);
			return load(match[1]);
		});
	assert.ok(records.length > 0);
	assert.equal(new Set(records.map((record) => record.enum_id)).size, records.length);
	assert.equal(new Set(records.map((record) => record.slug)).size, records.length);
	const keyBindings = records.flatMap((record) => record.bindings.key_value_types);
	const parameterBindings = records.flatMap((record) => record.bindings.scripting_parameter_types);
	assert.equal(new Set(keyBindings).size, keyBindings.length);
	assert.equal(new Set(parameterBindings).size, parameterBindings.length);
	const mission = records.find((record) => record.enum_id === 'MissionType');
	assert.equal(mission.values.find((value) => value.constant === 'MISSION_HUNT').value, 14);
	const action = records.find((record) => record.enum_id === 'ActionType');
	assert.equal(action.values.find((value) => value.constant === 'ACTION_DROP_POD').input, 'DropPod');
	assert.ok(records.every((record) => !Object.hasOwn(record, 'status') && record.source_files.length > 0));
});

test('scenario views merge selector-equivalent sections and preserve compatibility slugs', () => {
	const groups = scenarioGroups(keys);
	assert.ok(groups.length > 0);
	const map = groups.find((group) => group.slug === 'map');
	assert.ok(map);
	assert.equal(map.title, '[Map]');
	assert.ok(map.rows.some((row) => row.key === 'Fill'));
	assert.ok(map.rows.some((row) => row.key === 'Theater'));
	assert.equal(groups.some((group) => group.slug === 'name'), false);
	const houseGroup = groups.find((group) => group.slug === 'the-house-s-own-entry');
	assert.ok(houseGroup);
	assert.ok(houseGroup.rows.some((row) => row.key === 'NodeCount'));
	assert.equal(groups.some((group) => group.slug === 'the-base-owner-s-house-entry'), false);
	assert.deepEqual(Object.keys(referenceGroups(keys, 'rules')[0]).includes('rows'), true);
});

test('Other INI references are file-first, ordered, and use selector labels', () => {
	assert.deepEqual(OTHER_INI_FILES.map((file) => file.id), ['sun', 'campaigns', 'sounds', 'themes']);
	const groups = referenceGroups(keys, 'other');
	assert.ok(groups.length > 0);
	/* File-first is the contract, so the groups are required to follow the order the
	   file registry declares rather than a transcribed list of slugs. A section that
	   starts being extracted is then a registry edit, not a test edit. */
	const fileRank = new Map(OTHER_INI_FILES.flatMap(
		(file, index) => file.sourceFiles.map((name) => [name, index])));
	const ranks = groups.map((group) => fileRank.get(group.file));
	assert.ok(ranks.every((rank) => rank !== undefined), 'a group came from an unregistered file');
	assert.deepEqual(ranks, [...ranks].sort((left, right) => left - right));
	/* Within a file the registry's own order wins, and a section it does not name
	   still renders, so the slugs it does name have to stay in its relative order. */
	for (const file of OTHER_INI_FILES) {
		const named = (file.groupOrder ?? []).filter((slug) =>
			groups.some((group) => group.slug === slug));
		const rendered = groups
			.filter((group) => named.includes(group.slug))
			.map((group) => group.slug);
		assert.deepEqual(rendered, named, `${file.id} groups are out of registry order`);
	}
	assert.equal(groups.find((group) => group.slug === 'options').displayTitle, '[Options] in SUN.INI');
	assert.equal(groups.find((group) => group.slug === 'multiplayer').displayTitle, '[MultiPlayer] in SUN.INI');
	assert.equal(groups.find((group) => group.slug === 'campaign').displayTitle, 'Campaign sections in BATTLE*.INI');
	assert.equal(referenceFileLabel('sound01.ini'), 'SOUND.INI / SOUND01.INI');
	assert.equal(referenceFileLabel('theme01.ini'), 'THEME.INI + THEME01.INI');
	assert.deepEqual(keys.CD.scopes.find((scope) => scope.file === 'battle.ini').section,
		{ kind: 'identifier', source: 'campaign' });
	assert.deepEqual(keys.Priority.scopes.find((scope) => scope.file === 'sound01.ini').section,
		{ kind: 'identifier', source: 'sound' });
	assert.deepEqual(keys.Length.scopes.find((scope) => scope.file === 'theme01.ini').section,
		{ kind: 'identifier', source: 'theme' });
});

test('generated omission candidates stay in provenance and off the public scope shape', () => {
	const scopes = Object.values(keys).flatMap((entry) => entry.scopes);
	assert.ok(scopes.every((scope) => !Object.hasOwn(scope, 'default')));
	assert.ok(scopes.every((scope) => Object.hasOwn(scope._provenance, 'default_candidate')));
	assert.ok(scopes.some((scope) => scope._provenance.default_candidate === null));
	assert.ok(scopes.some((scope) => scope._provenance.default_candidate === ''));
	assert.ok(scopes.some((scope) => {
		const candidate = scope._provenance.default_candidate;
		return typeof candidate === 'string' && candidate !== '';
	}));
});

test('authored omission formatting and scope aggregation distinguish all public states', () => {
	const record = { key: 'Example', scopes: [{ route_id: 'one' }, { route_id: 'two' }] };
	const value = { kind: 'value', value: '' };
	const computed = { kind: 'computed', note: 'Derived from the section name.' };
	assert.deepEqual(formatWhenOmitted(value), { text: '(empty)', code: true, note: undefined });
	assert.deepEqual(formatWhenOmitted(computed), {
		text: 'Computed',
		code: false,
		note: 'Derived from the section name.',
	});
	assert.deepEqual(formatWhenOmitted(undefined), { text: 'Not documented', code: false });

	const first = [{ data: { key: 'Example', scope: 'one', when_omitted: value } }];
	assert.equal(whenOmittedForScope(first, record, record.scopes[0]), value);
	assert.equal(summarizeWhenOmitted([], record).text, 'Not documented');
	assert.equal(summarizeWhenOmitted(first, record).text, 'Partially documented');

	const differing = [
		...first,
		{ data: { key: 'Example', scope: 'two', when_omitted: computed } },
	];
	assert.equal(summarizeWhenOmitted(differing, record).text, 'Varies by type');

	const matching = [
		{ data: { key: 'Example', scope: 'one', when_omitted: { kind: 'value', value: '1' } } },
		{ data: { key: 'Example', scope: 'two', when_omitted: { kind: 'value', value: '1' } } },
	];
	assert.deepEqual(summarizeWhenOmitted(matching, record), { text: '1', code: true, note: undefined });
});

test('multi-scope and tombstone fixtures retain their distinct contracts', () => {
	/* Which key spans several files is extraction's business; that some key does, and
	   that each of its scopes stays separately addressable, is the contract. */
	const spanning = Object.entries(keys).filter(([, entry]) =>
		new Set(entry.scopes.map((scope) => scope.file)).size > 1);
	assert.ok(spanning.length > 0, 'no key spans more than one INI file');
	for (const [name, entry] of spanning) {
		assert.ok(
			entry.scopes.every((scope) => (scope.applies_to?.length ?? 0) > 0),
			`${name} publishes a scope with nothing to address it by`,
		);
	}
	const fixtureByType = new Map(tombstoneFixture.map((record) => [record.type, record]));
	assert.equal(fixtureByType.get('key').search_aliases[0], 'OldExample');
	assert.equal(fixtureByType.get('key').route, '/keys/oldexample/');
	assert.equal(fixtureByType.get('system').route, '/systems/old-system/');
	assert.deepEqual(fixtureByType.get('system').replacement, { type: 'system', id: 'drop-pods' });
	assert.equal(fixtureByType.get('command').route, '/commands/old-command/');
	assert.deepEqual(fixtureByType.get('command').replacement, { type: 'command', id: 'Follow' });
});

/* The gate builds with the fixtures so their pages are exercised; only the
   workflow builds without them. Both directions are settled here so a leak
   into a published artifact cannot wait on that build to be noticed. */
test('removed-entity fixtures are merged only while the fixture variable is set', () => {
	const committed = yaml('tombstones.yaml') ?? [];
	const routes = (rows) => rows.map((record) => record.route);

	const withoutFixtures = withFixtures(committed, tombstoneFixture, {});
	assert.deepEqual(withoutFixtures, committed);
	for (const route of routes(tombstoneFixture)) {
		assert.ok(!routes(withoutFixtures).includes(route), `${route} must stay out of a published build`);
	}

	const enabled = withFixtures(committed, tombstoneFixture, { MANUAL_TEST_FIXTURES: '1' });
	assert.deepEqual(routes(enabled), [...routes(committed), ...routes(tombstoneFixture)]);
	for (const route of ['/keys/oldexample/', '/systems/old-system/', '/commands/old-command/']) {
		assert.ok(routes(enabled).includes(route), `${route} must be exercised by the fixture build`);
	}
	assert.equal(fixturesEnabled({ MANUAL_TEST_FIXTURES: '0' }), false);
	assert.equal(fixturesEnabled({}), false);
});

test('structured scripting parameters preserve order and compound payloads', () => {
	const byId = (table, id) => table.find((row) => row.id === id);
	const names = (row) => row.parameters.map((parameter) => parameter.name);

	assert.deepEqual(names(byId(scripting.trigger_actions, 'TACTION_PLAY_ANIM')), ['Animation', 'Waypoint']);
	assert.deepEqual(names(byId(scripting.trigger_actions, 'TACTION_FLASH_TEAM')), ['Team', 'Duration']);
	assert.deepEqual(names(byId(scripting.trigger_events, 'TEVENT_NEAR_WAYPOINT')), ['Waypoint']);
	assert.deepEqual(names(byId(scripting.trigger_events, 'TEVENT_NONE')), []);
	assert.deepEqual(
		names(byId(scripting.team_missions, 'TMISSION_ATTACK_BUILDING_WITH_PROPERTY')),
		['Building type', 'Target property'],
	);
	assert.deepEqual(names(byId(scripting.team_missions, 'TMISSION_PLAY_ANIM')), ['Animation', 'Loop count']);

	/* The example line is generated from the row's own index and parameters, so it is
	   checked against them for every row rather than transcribed for a chosen few.
	   A parameter dropped from a line, or a line carrying another row's index, fails
	   wherever it happens instead of only in the sampled rows. */
	for (const table of [scripting.trigger_actions, scripting.trigger_events]) {
		for (const row of table) {
			const line = row.ini_example.line;
			assert.ok(
				line.startsWith(`<TriggerID>=1,${row.index},`),
				`${row.id} example does not lead with its own index`,
			);
			for (const parameter of row.parameters) {
				assert.ok(
					line.includes(`<${parameter.name}>`) || line.includes(`<${parameter.name} `),
					`${row.id} example omits the ${parameter.name} placeholder`,
				);
				// A rectangle is carried as four ordinary fields rather than one.
				if (parameter.type === 'rectangle') {
					for (const component of ['X', 'Y', 'width', 'height']) {
						assert.ok(
							line.includes(`<${parameter.name} ${component}>`),
							`${row.id} example omits the ${parameter.name} ${component} field`,
						);
					}
				}
			}
		}
	}
	assert.ok(scripting.trigger_actions.every((row) => row.ini_example?.section === '[Actions]'));
	assert.ok(scripting.trigger_events.every((row) => row.ini_example?.section === '[Events]'));
	assert.ok(scripting.team_missions.every((row) => !Object.hasOwn(row, 'ini_example')));

	for (const row of [
		...scripting.trigger_actions,
		...scripting.trigger_events,
		...scripting.team_missions,
	]) {
		assert.ok(row.need.startsWith('NEED_'));
		assert.ok(Array.isArray(row.parameters));
		assert.equal(Object.hasOwn(row, 'parameter'), false);
	}
});
