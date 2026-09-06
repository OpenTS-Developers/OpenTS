import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import { load } from 'js-yaml';

const commands = load(readFileSync(new URL('../../data/commands.yaml', import.meta.url), 'utf8'));
const all = [
	...commands.registered_commands,
	...commands.fixed_controls,
	...commands.launch_options,
];

test('registered command catalog reaches both builds', () => {
	assert.ok(commands.registered_commands.length > 0);
	assert.ok(commands.registered_commands.every((record) =>
		record.availability.builds.includes('release') && record.availability.builds.includes('debug')));
	assert.ok(commands.registered_commands.every((record) =>
		record.title.trim() && record.description.trim() && record.category.trim()));
});

test('command IDs and canonical routes are unique across every command kind', () => {
	assert.equal(new Set(all.map((record) => record.id)).size, all.length);
	assert.equal(new Set(all.map((record) => record.route_id)).size, all.length);
	assert.ok(all.every((record) => /^[a-z0-9]+(?:-[a-z0-9]+)*$/.test(record.route_id)));
});

test('team commands, forced bindings, fixed controls, and launch availability are preserved', () => {
	for (const prefix of ['TeamCreate', 'TeamSelect', 'TeamAddSelect', 'TeamAddTo', 'TeamCenter']) {
		assert.deepEqual(
			commands.registered_commands
				.filter((record) => record.id.startsWith(prefix + '_'))
				.map((record) => record.id),
			Array.from({ length: 10 }, (_, index) => `${prefix}_${index + 1}`),
		);
	}
	const forced = Object.fromEntries(commands.registered_commands
		.filter((record) => record.forced_binding)
		.map((record) => [record.id, record.forced_binding]));
	assert.deepEqual(forced, { Options: 'Escape', DeleteWaypoint: 'Delete' });
	assert.ok(commands.fixed_controls.some((record) => record.id === 'fixed:map-zoom'));
	assert.deepEqual(
		commands.launch_options.find((record) => record.id === 'launch:tournament-time').availability.builds,
		['release', 'debug'],
	);
});

test('generated commands never publish invented defaults or extraction-only fields', () => {
	assert.ok(all.every((record) => !Object.hasOwn(record, 'default_binding')));
	assert.ok(all.every((record) => Object.hasOwn(record, '_provenance')));
	assert.ok(commands.registered_commands.every((record) =>
		record.forced_binding === undefined || ['Delete', 'Escape'].includes(record.forced_binding)));
});
