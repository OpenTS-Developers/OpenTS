import './check-selector-render.mjs';

import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { resolve } from 'node:path';
import { UI } from '../src/i18n/en.mjs';
import { readCollection } from './lib/content-sources.mjs';
import {
	developmentRelease,
	escapeHtml,
	formatNavigation,
	proseNavigation,
	releaseExpectations,
	systemHubAnchors,
	systemNavigation,
	taxonomyContracts,
} from './lib/expectations.mjs';

const configuredBase = process.env.DOCS_BASE ?? '/OpenTS-Prepub-Scratch';
const base = configuredBase === '/' ? '' : '/' + configuredBase.replace(/^\/+|\/+$/g, '');
const isDemoPublication = base.toLowerCase() === '/docs-demo' || process.env.DOCS_DEMO === '1';
const fixturesEnabled = process.env.MANUAL_TEST_FIXTURES === '1';
const fixturePage = resolve('dist/keys/oldexample/index.html');
const inDevelopment = developmentRelease(base);
const releaseNavHref = 'href="' + base + '/changes/#' + inDevelopment.anchor + '"';

if (existsSync(fixturePage) !== fixturesEnabled) {
	throw new Error(fixturesEnabled
		? 'Fixture build is missing keys/oldexample/index.html'
		: 'Production build unexpectedly contains keys/oldexample/index.html');
}
for (const relative of ['systems/old-system/index.html', 'commands/old-command/index.html']) {
	const exists = existsSync(resolve('dist', relative));
	if (exists !== fixturesEnabled) {
		throw new Error(fixturesEnabled
			? `Fixture build is missing ${relative}`
			: `Production build unexpectedly contains ${relative}`);
	}
}

const cases = [
	['keys/aa/index.html', ['Specification', '<dt>Section</dt>', '[&lt;ObjectType ID&gt;]']],
	['keys/strength/index.html', ['<h2 id="scope-aircrafttype"', 'scope-overlaytype']],
	['keys/aaratio/index.html', ['This setting has no runtime effect', 'Parsed fraction that the engine never uses.', '.14']],
	['keys/armor/index.html', ['ArmorType', 'reference/enums/armor/']],
	['keys/image/index.html', ['scope-aircrafttype', 'scope-animtype', 'scope-tiberium', 'scope-buildingtype']],
	['reference/rules/buildingtype/index.html', ['data-reference-table', 'Filter keys', 'Settings read from']],
	['reference/art/buildingtype/index.html', ['Image-selected sections only', 'data-via-image="yes"', 'data-via-image="no"']],
	['reference/other/options/index.html', ['[Options] in SUN.INI', '<code>SUN.INI</code>']],
	['reference/other/campaign/index.html', ['Campaign sections in BATTLE*.INI', '[&lt;Campaign ID&gt;]']],
	['reference/other/sounds/index.html', ['Sound sections in SOUND.INI / SOUND01.INI', '[&lt;Sound ID&gt;]']],
	['reference/other/themes/index.html', ['Theme sections in THEME.INI + THEME01.INI', '[&lt;Theme ID&gt;]']],
	['mapping/scenario/basic/index.html', ['Scenario format', 'data-reference-table']],
	['mapping/actions/taction-change-house/index.html', ['Change House', 'Transfers every active object']],
	['mapping/actions/taction-play-anim/index.html', ['Parameters', 'Animation', 'Waypoint', 'INI example', '[Actions]', '&lt;TriggerID&gt;=1,41,0,&lt;Animation&gt;']],
	['mapping/missions/tmission-play-anim/index.html', ['Animation', 'Loop count']],
	['mapping/events/tevent-near-waypoint/index.html', ['Comes near waypoint', 'Behavior', 'INI example', '&lt;TriggerID&gt;=1,34,0,&lt;Waypoint&gt;']],
	['mapping/missions/tmission-loop/index.html', ['Jump to line', 'one-based']],
	['internals/class-hierarchy/index.html', ['Object and type system', 'Primary runtime hierarchy', 'Type-definition hierarchy', 'AbstractTypeClass', 'code/abstype.h']],
	['internals/radio/index.html', ['Radio contact protocol', 'Contact state', 'Messages and responses', 'Compute_CRC', 'code/radio.cpp']],
	['internals/locomotion/index.html', ['Locomotion and piggybacking', 'FootClass::Locomotion', 'IPiggyback', 'Piggyback_CLSID']],
	['reference/enums/mission/index.html', ['data-enum-table', 'MISSION_HUNT', 'Stored value', 'Used by', 'TMISSION_DO']],
	['systems/drop-pods/index.html', ['ots-page-subtitle', 'Entry paths', 'Approach and descent', 'Touchdown']],
	['systems/base-adjacency/index.html', ['ots-page-subtitle', 'Base placement and adjacency', 'Placement decision order', 'Adjacent']],
	['reference/map-seeds/index.html', ['[RandomMap]', '<strong data-visible-count>', 'Map seed definition']],
	['reference/theater-controls/index.html', ['[General]', '[&lt;TileSet ID&gt;]', '<strong data-visible-count>']],
	['using/project-status/index.html', ['Project status', 'Toolchain and targets', 'Authoritative files', 'docs/BUILDING.md']],
	['commands/follow/index.html', ['Command', 'Hotkey command', 'KEYBOARD.INI', 'Release and Debug']],
	['using/command-line/help/index.html', ['Command-line help', 'Syntax', 'Aliases', 'Release and Debug', 'ots-breadcrumb']],
	['commands/fixed-map-zoom/index.html', ['Map zoom', 'Fixed control', 'Fixed controls', 'ots-breadcrumb']],
	['formats/mix/index.html', ['MIX archives', 'Binary format', '<dt>Role</dt>', 'code/ccfile.cpp', 'Source files', 'code/mixfile.h']],
	['changes/opents-manual/index.html', ['OpenTS manual', 'Released in', '0.1.0', 'reference pages for every']],
	['changes/fastmath-runtime/index.html', ['Replace the fastmath lookup tables', 'numerical results may differ', 'same OpenTS version']],
	['changes/cd-search-path/index.html', ['Remove CD-ROM-dependent startup behavior', 'local data search path', 'PlayIntro']],
	['changes/modem-play/index.html', ['Remove modem and null-modem play', 'SerialDefaults', 'keep their stored values']],
];

const renderedMain = (relative) => {
	const html = readFileSync(resolve('dist', relative), 'utf8');
	const main = html.match(/<main\b[\s\S]*<\/main>/)?.[0];
	if (!main) throw new Error(`Rendered page has no main content: ${relative}`);
	return main;
};

const assertOrdered = (source, needles, label) => {
	let cursor = -1;
	for (const needle of needles) {
		const next = source.indexOf(needle, cursor + 1);
		if (next < 0) throw new Error(`${label} is missing ${JSON.stringify(needle)}`);
		cursor = next;
	}
};
const renderedSearchScopes = (relative) => {
	const html = readFileSync(resolve('dist', relative), 'utf8');
	return [...html.matchAll(/data-pagefind-filter="([^"]+)"/g)].map((match) => match[1]).sort();
};

/* Every page whose search taxonomy comes from its own frontmatter is checked,
   because a page reaching the head without its record loses its Topic filter
   with nothing else to show for it. Routes whose scopes follow from the path
   alone are settled dist-free in tests/search-scopes.test.mjs; repeating them
   here would only compare searchScopesForPath against itself, so one page
   stands as proof that the head emits the markers at all. The rows below it
   are samples of the taxonomies that live in generated data. */
const searchScopeContracts = [
	['index.html', ['Part of the manual:Start']],
	...taxonomyContracts('systems', 'systems', UI.systemCategories, UI.navigation.systems),
	...taxonomyContracts('internals', 'internals', UI.internalCategories, UI.navigation.internals),
	...taxonomyContracts('using', 'using', UI.usingCategories, UI.navigation.using),
	['formats/mix/index.html', ['Topic:Binary format', 'Part of the manual:Formats']],
	['commands/follow/index.html', ['Topic:Player commands', 'Topic:Hotkey commands', 'Part of the manual:Commands']],
	['using/command-line/help/index.html', ['Topic:Command line options', 'Topic:Developer commands', 'Part of the manual:Using OpenTS']],
	['keys/bounceanim/index.html', ['Topic:Art', 'Topic:Rules', 'Part of the manual:INI reference']],
	['keys/action/index.html', ['Topic:Rules', 'Topic:Scenario format', 'Part of the manual:INI reference', 'Part of the manual:Mapping']],
	['mapping/actions/taction-change-house/index.html', ['Topic:Trigger actions', 'Part of the manual:Mapping']],
	['mapping/team-types/index.html', ['Topic:Registry', 'Topic:Team types', 'Part of the manual:Formats', 'Part of the manual:Mapping']],
];
if (fixturesEnabled) {
	searchScopeContracts.push(['keys/oldexample/index.html', ['Part of the manual:INI reference']]);
}
for (const [relative, expected] of searchScopeContracts) {
	const actual = renderedSearchScopes(relative);
	const sortedExpected = [...expected].sort();
	if (JSON.stringify(actual) !== JSON.stringify(sortedExpected)) {
		throw new Error(`${relative} search scopes are ${actual.join(', ')}; expected ${sortedExpected.join(', ')}`);
	}
}

const indexContracts = [
	['index.html',
		['Setup, runtime behavior, configuration, mapping, and source-level internals.', 'Using OpenTS', 'Features & systems', 'All INI keys', 'Commands', 'Formats', '>Modding</h2>', '>Engine development</h2>', 'Engine internals', 'sl-link-card'],
		['INI files, map data, scripting, engine systems, and versioned changes.', 'Browse the manual', 'Choose your route', 'Why the reference is trustworthy', '>Documentation</h2>', 'INI key reference']],
	/* Which categories and pages appear is asserted below, against the content
	   tree, so that filling an empty category is not mistaken for a regression.
	   This entry is a sample proving the lede and group headings render. */
	['systems/index.html',
		['Runtime behavior and the settings, formats, and commands that control it.', '>Combat &amp; weapons</h2>', '>Units, buildings &amp; economy</h2>', '>Scenarios &amp; AI</h2>', '>Interface &amp; runtime</h2>', 'ots-category-grid'],
		['Engine behavior grouped by subsystem', 'Engine mechanics, traced from behavior', 'No authored overview yet']],
	['guides/index.html',
		['Procedures and failure cases for common modding tasks.'],
		['Practical walkthroughs', 'No guides are available.', 'Base placement and adjacency']],
	['using/index.html',
		['Developer-build setup, local data, configuration, and save-game behavior.', 'Getting started', 'Configuration', 'Compatibility &amp; migration', 'Troubleshooting', 'Project status', 'Command line options', 'sl-link-card'],
		['compatibility boundaries']],
	['commands/index.html',
		['Hotkey commands bound through', 'KEYBOARD.INI', 'data-command-index', 'Filter commands', 'Player commands', 'Debug only'],
		['invented default', 'Rebindable command', 'Launch option', 'Command line option', 'command line options accepted', 'Fixed control']],
	['commands/fixed-controls/index.html',
		['Controls with hardcoded keys', 'data-command-index', 'Filter controls', 'ots-breadcrumb', 'Map zoom', 'Debug only'],
		['All types', 'Filter commands', 'Hotkey command', 'Command line option', 'fixed:']],
	['using/command-line/index.html',
		['Options accepted on the command line.', 'data-command-index', 'Filter options', 'ots-breadcrumb', 'Command-line help', 'Windowed mode', 'Debug only'],
		['All types', 'Filter commands', 'Hotkey command', 'Fixed control', 'launch:']],
	['formats/index.html',
		['File syntax, registration, record layout, and binary data.', 'Syntax', 'Configuration file', 'Registry', 'Record', 'Binary format', 'MIX archives', 'Map seed files', 'sl-link-card'],
		['byte layout', 'compatibility boundaries', 'Map seed file format', 'Theater control file format']],
	['reference/index.html',
		['INI settings grouped by name, file, section, and object type.', 'All INI keys', 'sl-link-button', '>Files</h2>', 'Other INI files', 'Map seed files', 'Theater controls', 'Enums'],
		['Find a setting by name', 'Browse by file', 'distinct settings', 'documented scopes', '100%', 'Maps and AI', 'Search by key name']],
	['reference/all/index.html',
		['Every accepted INI key, searchable by name, description, value type, section, and file family.', 'ots-breadcrumb', 'data-reference-table'],
		['Search by key, behavior', 'The complete active key surface.']],
	['reference/rules/index.html',
		['Object types', 'Behavior settings', 'Difficulty settings', 'Named sections', '[General]', '[AI]', '[CombatDamage]', 'data-compact-index'],
		['Object and entry types', 'Global settings', 'Global sections', 'Search all INI keys', 'By effective type', 'global rules</a>', 'difficulty settings</a>', 'mission behavior</a>']],
	['reference/art/index.html',
		['Object types', 'data-compact-index'],
		['Object and entry types', 'Search all INI keys', 'By effective type', 'Global sections']],
	['reference/other/index.html',
		['SUN.INI', 'Options file', 'Load behavior:', 'data-compact-index'],
		['Loading:', 'Search all INI keys']],
	['internals/index.html',
		['Source-level descriptions', 'Architecture', 'Object model, type system, and engine-wide structural invariants.', 'Object and type system', 'Simulation systems', 'Locomotion and piggybacking', 'Radio contact protocol', 'sl-link-card'],
		['Source-oriented guides', 'Class hierarchy', 'Radio system']],
	['reference/enums/index.html',
		['Fixed engine value sets', 'Mission', 'Armor', 'Runtime object type', 'data-compact-index'],
		['houses', 'themes', 'sounds', 'movies', 'Side slot', 'Superweapon slot', 'Voxel animation type']],
	['mapping/index.html',
		['Map sections, trigger records, and AI team definitions.', 'Scenario data', 'Triggers', 'AI teams', 'Team script missions', 'sl-link-card'],
		['Map files, trigger logic', 'generated section views', 'conditions</span>', 'effects</span>', 'orders</span>', 'Team missions</']],
	['mapping/scenario/index.html',
		['Named sections and keys read from map files.', '[Basic]', '[&lt;House ID&gt;]', 'data-compact-index'],
		['Named sections and settings', 'Named and positional formats differ', 'Cross-referenced structures', 'Extracted named sections', 'sl-link-card', '[]</', 'Base owner House ID']],
	['mapping/actions/index.html',
		['Actions executed after a trigger&#39;s events are satisfied.', 'data-scripting-table'],
		['Effects a trigger executes', 'Select an entry for parameters']],
	['mapping/team-types/index.html',
		['Registers team definitions that link an owner, TaskForce, Script, waypoint, and behavior flags.', 'Registration sections', '[TeamTypes]', 'data-reference-table'],
		['A TeamType links a TaskForce and Script', 'A TeamType connects', 'A TeamType joins one TaskForce']],
	['changes/index.html',
		['>Changes</h1>', 'Versioned behavior and compatibility changes, newest release first.', 'ots-release-badge', 'sl-badge', '0.1.0', 'Upgrade to 0.1.0', 'data-change-list'],
		["What's new", 'A permanent, versioned record of deliberate OpenTS changes.']],
	['changes/0.1.0/index.html',
		['Upgrade to 0.1.0', 'Released', 'migration before existing content is used with this version.', 'Serialize save games member by member', 'View all changes for 0.1.0'],
		['no recorded migration steps']],
];

for (const [relative, expected, retired] of indexContracts) {
	const main = renderedMain(relative);
	for (const text of expected) {
		if (!main.includes(text)) throw new Error(`${relative} does not contain ${JSON.stringify(text)}`);
	}
	for (const text of retired) {
		if (main.includes(text)) throw new Error(`${relative} still contains retired copy ${JSON.stringify(text)}`);
	}
}

/* A system category appears on the hub exactly when it holds a page. Asserting
   that against the content tree rather than against a written-out list means a
   category filling up is not reported as a regression, which is what a list of
   absent headings used to do. */
const authoredSystemCategories = new Set(
	readdirSync(resolve('../content/systems'))
		.filter((name) => name.endsWith('.md'))
		.map((name) => readFileSync(resolve('../content/systems', name), 'utf8'))
		.map((text) => text.match(/^category:\s*(\S+)\s*$/m)?.[1])
		.filter(Boolean));
const renderedSystemCategories = new Set(
	[...renderedMain('systems/index.html').matchAll(/id="category-([a-z0-9-]+)"/g)].map((match) => match[1]));
const missingCategories = [...authoredSystemCategories].filter((id) => !renderedSystemCategories.has(id)).sort();
const strayCategories = [...renderedSystemCategories].filter((id) => !authoredSystemCategories.has(id)).sort();
if (missingCategories.length || strayCategories.length) {
	throw new Error('systems/index.html categories disagree with content/systems: '
		+ `${missingCategories.length ? `authored but not rendered: ${missingCategories.join(', ')}. ` : ''}`
		+ `${strayCategories.length ? `rendered but holding no page: ${strayCategories.join(', ')}. ` : ''}`
		+ 'Adding the first page to a category is expected to change this; a category '
		+ 'losing its last page is not.');
}
if (!authoredSystemCategories.size) throw new Error('No system categories were read from content/systems');

/* The hub says either that there are no guides or what they are, and which one
   it should say follows from the collection rather than from this list. */
const guides = readCollection('guides');
const guidesHub = renderedMain('guides/index.html');
const emptyGuides = 'The manual contains no guides yet.';
if (guides.length === 0) {
	if (!guidesHub.includes(emptyGuides)) throw new Error('Guides hub does not explain that it is empty');
} else {
	if (guidesHub.includes(emptyGuides)) {
		throw new Error('Guides hub still says it is empty while guides are authored');
	}
	for (const guide of guides) {
		if (!guidesHub.includes(`href="${base}/guides/${guide.id}/"`)) {
			throw new Error(`Guides hub does not link ${guide.id}`);
		}
	}
}

for (const relative of [
	'reference/rules/index.html',
	'reference/art/index.html',
	'reference/other/index.html',
	'mapping/index.html',
	'mapping/scenario/index.html',
]) {
	const main = renderedMain(relative);
	if (/\b\d+\s+(?:groups|settings|conditions|effects|orders)\b/i.test(main)) {
		throw new Error(`${relative} still contains a navigation count`);
	}
}

/* The count beside a table states how many rows it is showing, and both pages
   already refuse to build if the extraction stops producing them. What is worth
   holding is that the two agree, which a written-out number cannot say. */
for (const relative of ['reference/map-seeds/index.html', 'reference/theater-controls/index.html']) {
	const frames = renderedMain(relative)
		.split('ots-filter-table')
		.slice(1)
		.filter((frame) => frame.includes('<strong data-visible-count>'));
	if (!frames.length) throw new Error(`${relative} renders no counted table`);
	for (const frame of frames) {
		const stated = Number(frame.match(/<strong data-visible-count>(\d+)<\/strong>/)?.[1]);
		const rendered = frame.split('<tr data-row').length - 1;
		if (!Number.isInteger(stated) || stated !== rendered) {
			throw new Error(`${relative} states ${stated} rows but renders ${rendered}`);
		}
	}
}

const artGroupMain = renderedMain('reference/art/buildingtype/index.html');
if (artGroupMain.includes('One effective type view')) {
	throw new Error('Art group still renders the retired effective-type callout');
}
const abstractRulesMain = renderedMain('reference/rules/abstracttype/index.html');
if (abstractRulesMain.includes('AITriggerType, AircraftType, AnimType')) {
	throw new Error('Rules group still renders the long generated applicability list');
}
const armorMain = renderedMain('keys/armor/index.html');
if (!armorMain.includes('ObjectType and derived types') || !armorMain.includes('<details class="ots-type-coverage">')) {
	throw new Error('Key detail does not expose condensed and expandable applicability');
}
const strengthMain = renderedMain('keys/strength/index.html');
if (strengthMain.includes('meanings') || strengthMain.includes('ots-scope-jump')) {
	throw new Error('Strength still renders the retired multi-meaning jump box');
}

const dropPodsHtml = readFileSync(resolve('dist/systems/drop-pods/index.html'), 'utf8');
const dropPodsMain = renderedMain('systems/drop-pods/index.html');
const expectedDropPodHeadings = [
	['2', 'entry-paths', 'Entry paths'],
	['3', 'drop-pods-superweapon', 'Drop Pods superweapon'],
	['3', 'droppod-teamtype', 'Droppod TeamType'],
	['2', 'approach-and-descent', 'Approach and descent'],
	['3', 'approach-selection', 'Approach selection'],
	['3', 'descent-and-airborne-effects', 'Descent and airborne effects'],
	['2', 'touchdown', 'Touchdown'],
];
const expectedDropPodIds = new Set(expectedDropPodHeadings.map(([, id]) => id));
const renderedDropPodHeadings = [
	...dropPodsMain.matchAll(
		/<div class="sl-heading-wrapper level-h([23])"><h\1 id="([^"]+)">([^<]+)<\/h\1>/g,
	),
]
	.map((match) => match.slice(1, 4))
	.filter(([, id]) => expectedDropPodIds.has(id));
if (JSON.stringify(renderedDropPodHeadings) !== JSON.stringify(expectedDropPodHeadings)) {
	throw new Error(
		'Drop pods heading hierarchy is ' + JSON.stringify(renderedDropPodHeadings)
		+ '; expected ' + JSON.stringify(expectedDropPodHeadings),
	);
}
for (const [rank, id] of expectedDropPodHeadings) {
	const expectedDepth = rank === '2' ? '0' : '1';
	const tocLink = new RegExp(
		`<a\\b(?=[^>]*\\bhref="#${id}")(?=[^>]*\\bstyle="[^"]*--depth:\\s*([01]);)[^>]*>`,
		'g',
	);
	const depths = [
		...dropPodsHtml.matchAll(tocLink),
	].map((match) => match[1]);
	if (depths.length === 0 || depths.some((depth) => depth !== expectedDepth)) {
		throw new Error(`Drop pods TOC depth for #${id} is ${depths.join(',')}; expected ${expectedDepth}`);
	}
}
for (const retired of ['<h2 id="teamtype-delivery">', '<h2 id="trajectory">', '<h2 id="delivery-sequence">']) {
	if (dropPodsMain.includes(retired)) throw new Error(`Drop pods still renders retired top-level heading ${retired}`);
}
if (!dropPodsMain.includes(`href="${base}/internals/locomotion/#piggybacking"`)) {
	throw new Error('Drop pods does not contextually link its first piggyback use to the locomotion internal');
}

const locomotionMain = renderedMain('internals/locomotion/index.html');
if (!locomotionMain.includes('<h2 id="piggybacking">Piggybacking</h2>')
	|| !locomotionMain.includes('Begin_Piggyback')
	|| !locomotionMain.includes('End_Piggyback')) {
	throw new Error('Locomotion internal is missing the canonical piggyback anchor or operations');
}

const imageHtml = readFileSync(resolve('dist/keys/image/index.html'), 'utf8');
const imageMain = renderedMain('keys/image/index.html');
const imageScopes = [
	['aircrafttype', 'Image ID'],
	['animtype', 'Animation Image ID'],
	['tiberium', 'Tiberium overlay variant'],
	['buildingtype', 'Building main shape'],
];
assertOrdered(
	imageMain,
	imageScopes.map(([scope, label]) => `<h2 id="scope-${scope}">${label}</h2>`),
	'Image scope fragments',
);
for (const [scope] of imageScopes) {
	if (!imageHtml.includes(`href="#scope-${scope}"`)) {
		throw new Error(`Image TOC is missing the stable #scope-${scope} fragment`);
	}
}

function renderedReferenceRow(relative, keyRoute) {
	const main = renderedMain(relative);
	const href = `href="${base}/keys/${keyRoute}/"`;
	const row = [...main.matchAll(/<tr\b[^>]*>[\s\S]*?<\/tr>/g)]
		.map((match) => match[0])
		.find((candidate) => candidate.includes(href));
	if (!row) throw new Error(`${relative} has no row for ${href}`);
	return row;
}

const rulesImageRow = renderedReferenceRow('reference/rules/objecttype/index.html', 'image');
if (!rulesImageRow.includes('buildingtype') || rulesImageRow.includes('Except BuildingType')) {
	throw new Error('Rules ObjectType Image row does not include BuildingType directly');
}
const animImageRow = renderedReferenceRow('reference/art/animtype/index.html', 'image');
if (!animImageRow.includes('data-via-image="no"')
	|| !animImageRow.toLowerCase().includes('[<objecttype id>]')) {
	throw new Error('AnimType Image row is not the direct Art ObjectType-ID scope');
}
const buildingImageRow = renderedReferenceRow('reference/art/buildingtype/index.html', 'image');
if (!buildingImageRow.includes('data-via-image="yes"')
	|| !buildingImageRow.toLowerCase().includes('[<image id>]')) {
	throw new Error('BuildingType Image row is not the additive Image-ID main-shape scope');
}

/* A row is badged from what it stands for. A row for one scope answers
   definitely; a row for a whole spelling keeps the partly inert case distinct,
   so Armor is never presented as a setting that does nothing. */
const noEffectBadge = '<span class="ots-badge ots-badge-noeffect">No effect</span>';
const partlyNoEffectBadge = '<span class="ots-badge ots-badge-noeffect">No effect in some scopes</span>';
for (const [relative, keyRoute, state, badge] of [
	['reference/all/index.html', 'aaratio', 'none', noEffectBadge],
	['reference/all/index.html', 'armor', 'partial', partlyNoEffectBadge],
	['reference/art/animtype/index.html', 'armor', 'none', noEffectBadge],
	['reference/rules/objecttype/index.html', 'armor', 'used', ''],
	['reference/rules/housetype/index.html', 'armor', 'used', ''],
	['reference/rules/difficulty-settings/index.html', 'armor', 'used', ''],
]) {
	const row = renderedReferenceRow(relative, keyRoute);
	if (!row.includes(`data-effect-value="${state}"`)) {
		throw new Error(`${relative} row for ${keyRoute} is not marked ${state}`);
	}
	if (badge ? !row.includes(badge) : row.includes('ots-badge-noeffect')) {
		throw new Error(`${relative} row for ${keyRoute} carries the wrong no-effect badge`);
	}
}

const allKeysMain = renderedMain('reference/all/index.html');
for (const control of ['<select data-effect>', '>Runtime effect<', '>Used by the game<', '>No effect</option>']) {
	if (!allKeysMain.includes(control)) {
		throw new Error(`All INI keys is missing the runtime-effect filter fragment ${JSON.stringify(control)}`);
	}
}

/* Key pages carry the same marker into search results. */
for (const [relative, marker] of [
	['keys/aaratio/index.html', '<meta data-pagefind-meta="setting:No effect">'],
	['keys/armor/index.html', '<meta data-pagefind-meta="setting:No effect in some scopes">'],
]) {
	if (!readFileSync(resolve('dist', relative), 'utf8').includes(marker)) {
		throw new Error(`${relative} does not publish ${JSON.stringify(marker)} to search`);
	}
}
for (const relative of ['keys/aa/index.html', 'keys/basenormal/index.html']) {
	if (readFileSync(resolve('dist', relative), 'utf8').includes('data-pagefind-meta="setting:')) {
		throw new Error(`${relative} marks a live setting as having no effect`);
	}
}

/* The AI-assistance notice is site-level: one page carries it, never a page
   about one entity. The effect filter appears only where a table has something
   to filter. */
const aiAssistanceNotice = 'This manual was produced with AI assistance and may contain imprecisions.';
const filterTableRoot = /<div class="[^"]*\bots-filter-table\b[^"]*" data-reference-table>[\s\S]*?<p class="ots-empty"/g;
const aiNoticePages = [];
for (const relative of readdirSync(resolve('dist'), { recursive: true })) {
	const path = String(relative).split('\\').join('/');
	if (!path.endsWith('.html')) continue;
	const html = readFileSync(resolve('dist', relative), 'utf8');
	if (html.includes(aiAssistanceNotice)) aiNoticePages.push(path);
	for (const [table] of html.matchAll(filterTableRoot)) {
		const inert = table.includes('data-effect-value="none"') || table.includes('data-effect-value="partial"');
		if (inert !== table.includes('<select data-effect>')) {
			throw new Error(inert
				? `${path} hides the runtime-effect filter from a table that lists an inert setting`
				: `${path} offers the runtime-effect filter on a table with nothing to filter`);
		}
	}
}
if (aiNoticePages.length !== 1 || aiNoticePages[0] !== 'index.html') {
	throw new Error(`The AI-assistance notice must appear on the landing page alone; found on ${aiNoticePages.join(', ') || 'no page'}`);
}

const retiredBuildingException = readdirSync(resolve('dist'), { recursive: true })
	.filter((path) => path.endsWith('.html'))
	.find((path) => readFileSync(resolve('dist', path), 'utf8').includes('Except BuildingType'));
if (retiredBuildingException) {
	throw new Error(`Retired "Except BuildingType" copy remains in ${retiredBuildingException}`);
}

const mappingMain = renderedMain('mapping/index.html');
for (const route of ['scenario', 'events', 'actions', 'team-types', 'task-forces', 'scripts', 'missions', 'ai-triggers']) {
	const href = `href="${base}/mapping/${route}/"`;
	if (mappingMain.split(href).length !== 2) throw new Error(`Mapping hub must link ${route} exactly once`);
}
const keysRedirect = readFileSync(resolve('dist/keys/index.html'), 'utf8');
if (!/<html\b[^>]*\bdata-pagefind-ignore\b/i.test(keysRedirect)) {
	throw new Error('/keys/ redirect is not excluded from Pagefind');
}
if (!/<meta\b[^>]*\bname=["']robots["'][^>]*\bcontent=["']noindex["']/i.test(keysRedirect)) {
	throw new Error('/keys/ redirect is not marked noindex');
}
for (const marker of [
	`http-equiv="refresh" content="0; url=${base}/reference/all/"`,
	`rel="canonical" href="${base}/reference/all/"`,
]) {
	if (!keysRedirect.includes(marker)) throw new Error(`/keys/ redirect is missing ${JSON.stringify(marker)}`);
}

const otherIndex = readFileSync(resolve('dist/reference/other/index.html'), 'utf8');
if (!otherIndex.includes('data-manual-view-selector') || !otherIndex.includes('data-active-view="reference"')) {
	throw new Error('Reference page does not render the reference view selector state');
}
if (!otherIndex.includes('data-manual-view-nav="reference"')) {
	throw new Error('Reference page does not render its local view navigation');
}

const selectorStart = otherIndex.indexOf('data-manual-view-selector');
const localNavStart = otherIndex.indexOf('data-manual-view-nav');
assertOrdered(
	otherIndex.slice(selectorStart, localNavStart),
	[`href="${base}/"`, '>Modding</p>', '/using/', '/systems/', '/reference/', '/guides/', '/mapping/', '/commands/', '/formats/', '>Engine development</p>', '/internals/', '/changes/'],
	'Manual view selector',
);
assertOrdered(
	otherIndex.slice(localNavStart),
	['/reference/rules/', '/reference/art/', '/reference/other/', '/reference/map-seeds/', '/reference/theater-controls/', '/reference/enums/'],
	'INI Reference navigation',
);

/* The root page is a splash template without a sidebar; the Start highlight
   is asserted on the first regular Start-view page instead. */
for (const relative of ['how-to-read/index.html']) {
	const startPage = readFileSync(resolve('dist', relative), 'utf8');
	if (!startPage.includes('data-active-view="start"')) {
		throw new Error(`${relative} does not mark the Start view as active`);
	}
	const startSelector = startPage.slice(
		startPage.indexOf('data-manual-view-selector'),
		startPage.indexOf('data-manual-view-nav'),
	);
	if (!/<a[^>]*href="[^"]*\/"[^>]*class="[^"]*active[^"]*"[^>]*aria-current="true"/.test(startSelector)
		&& !/<a[^>]*class="[^"]*active[^"]*"[^>]*aria-current="true"[^>]*href="[^"]*\/"/.test(startSelector)) {
		throw new Error(`${relative} does not highlight the Start selector entry`);
	}
}

function renderedSystemNavigation(relative) {
	const html = readFileSync(resolve('dist', relative), 'utf8');
	const marker = 'data-manual-view-nav="systems"';
	const start = html.indexOf(marker);
	const end = html.indexOf('</nav>', start);
	if (start < 0 || end < 0) {
		throw new Error(`${relative} does not contain the Features & systems local navigation`);
	}
	return html.slice(start, end);
}

function systemNavigationLinks(navigation) {
	return [...navigation.matchAll(/<a\b[^>]*\bdata-system-nav-kind="[^"]+"[^>]*>/g)].map(([tag]) => {
		const attribute = (name) => tag.match(new RegExp(`\\b${name}="([^"]*)"`))?.[1];
		return {
			kind: attribute('data-system-nav-kind'),
			id: attribute('data-system-nav-id'),
			href: attribute('href'),
			current: attribute('aria-current') === 'page',
		};
	});
}

const systemsIndexNavigation = renderedSystemNavigation('systems/index.html');
const dropPodsNavigation = renderedSystemNavigation('systems/drop-pods/index.html');
const expectedSystemNavigation = systemNavigation(base);

for (const [relative, navigation, currentId] of [
	['systems/index.html', systemsIndexNavigation, 'overview'],
	['systems/drop-pods/index.html', dropPodsNavigation, 'drop-pods'],
]) {
	if (!navigation.includes('data-system-navigation')) {
		throw new Error(`${relative} does not use the dedicated Features & systems navigation tree`);
	}
	if (navigation.includes('<details')) {
		throw new Error(`${relative} collapses the Features & systems navigation tree`);
	}
	if (navigation.includes('old-system')) {
		throw new Error(`${relative} includes a system tombstone in active navigation`);
	}

	const links = systemNavigationLinks(navigation);
	const actual = links.map(({ kind, id, href }) => `${kind}:${id}:${href}`).join('|');
	const expected = expectedSystemNavigation
		.map(([kind, id, href]) => `${kind}:${id}:${href}`)
		.join('|');
	if (actual !== expected) {
		throw new Error(`${relative} systems navigation is ${actual}; expected ${expected}`);
	}

	const current = links.filter((link) => link.current);
	if (current.length !== 1 || current[0].id !== currentId) {
		throw new Error(`${relative} current systems navigation item is not ${currentId}`);
	}
}

if (!/data-system-nav-kind="group" data-system-nav-id="combat"[^>]*>[\s\S]*?data-system-nav-kind="category" data-system-nav-id="superweapons-special"[^>]*>[\s\S]*?<\/a><ul\b[^>]*><li\b[^>]*><a\b[^>]*data-system-nav-kind="system" data-system-nav-id="drop-pods"/.test(dropPodsNavigation)) {
	throw new Error('Drop pods is not nested under its populated category and group');
}

const systemsHubMain = renderedMain('systems/index.html');
for (const target of systemHubAnchors(base)) {
	if (!systemsHubMain.includes(target)) {
		throw new Error(`Features & systems navigation target ${target} is missing`);
	}
}
for (const entry of readCollection('systems')) {
	const href = `href="${base}/systems/${entry.id}/"`;
	if (systemsHubMain.split(href).length !== 2) {
		throw new Error(`systems/index.html must link ${entry.id} exactly once`);
	}
}

function renderedLocalNavigation(relative, view) {
	const html = readFileSync(resolve('dist', relative), 'utf8');
	const marker = `data-manual-view-nav="${view}"`;
	const start = html.indexOf(marker);
	const end = html.indexOf('</nav>', start);
	if (start < 0 || end < 0) {
		throw new Error(`${relative} does not contain the ${view} local navigation`);
	}
	return html.slice(start, end);
}

const expectedInternalNavigation = proseNavigation('internals', '/internals', base)
	.map((href) => `href="${href}"`);
for (const relative of [
	'internals/locomotion/index.html',
	'internals/radio/index.html',
]) {
	const navigation = renderedLocalNavigation(relative, 'internals');
	assertOrdered(navigation, expectedInternalNavigation, `${relative} Internals navigation`);
	for (const href of expectedInternalNavigation.slice(1)) {
		if (navigation.split(href).length !== 2) {
			throw new Error(`${relative} must contain ${href} exactly once in the Internals navigation`);
		}
	}
}
const expectedFormatNavigation = formatNavigation(base);
for (const relative of [
	'formats/mix/index.html',
	'formats/ini-syntax/index.html',
]) {
	const navigation = renderedLocalNavigation(relative, 'formats');
	assertOrdered(navigation, expectedFormatNavigation, `${relative} Formats navigation`);
	for (const needle of expectedFormatNavigation) {
		if (navigation.split(needle).length !== 2) {
			throw new Error(`${relative} must contain ${needle} exactly once in the Formats navigation`);
		}
	}
}

const locomotionNavigation = renderedLocalNavigation(
	'internals/locomotion/index.html',
	'internals',
);
const locomotionNavigationLink = [...locomotionNavigation.matchAll(/<a\b[^>]*>[\s\S]*?<\/a>/g)]
	.map((match) => match[0])
	.find((link) => link.includes(`href="${base}/internals/locomotion/"`));
if (!locomotionNavigationLink?.includes('aria-current="page"')) {
	throw new Error('Locomotion is not the current Internals navigation entry on its own page');
}

const otherMain = renderedMain('reference/other/index.html');
for (const text of [
	'SUN.INI', 'Options file', 'BATTLE*.INI', 'Campaign definitions',
	'SOUND.INI / SOUND01.INI', 'Sound effects', 'THEME.INI + THEME01.INI', 'Music tracks',
]) {
	if (!otherMain.includes(text)) throw new Error(`Other INI landing does not contain ${JSON.stringify(text)}`);
}
if (otherMain.includes('Global sections') || otherMain.includes('By effective type')) {
	throw new Error('Other INI landing still uses the retired generic grouping labels');
}

if (fixturesEnabled) {
	cases.push(
		['keys/oldexample/index.html', ['Removed entity', 'OldExample', 'Historical identity']],
		['systems/old-system/index.html', ['Removed entity', 'OldSystem', 'Historical identity', 'drop-pods']],
		['commands/old-command/index.html', ['Removed entity', 'OldCommand', 'Historical identity', 'Follow']],
	);
}

const baselineAction = readFileSync(
	resolve('dist/mapping/actions/taction-play-anim/index.html'),
	'utf8',
);
if (baselineAction.includes('Added in') || baselineAction.includes('<h2 id="entity-history">History</h2>')) {
	throw new Error('Vanilla-baseline action received invented lifecycle history');
}

for (const [relative, href, label] of [
	['mapping/missions/tmission-attack/index.html', '/mapping/scripts/', 'Scripts'],
	['keys/droppodheight/index.html', '/systems/drop-pods/', 'Drop pods'],
	['keys/basenormal/index.html', '/systems/base-adjacency/', 'Base placement and adjacency'],
]) {
	const main = renderedMain(relative);
	if (!main.includes('<h2 id="related-resources">Related resources</h2>')
		|| !main.includes(`href="${base}${href}"`)
		|| !main.includes(`>${label}</a>`)) {
		throw new Error(`${relative} is missing the reverse link to ${label}`);
	}
}

const relatedMain = renderedMain('keys/basenormal/index.html');
if (!/<section class="ots-related-resources"[^>]*><h2 id="related-resources">Related resources<\/h2><ul>/.test(relatedMain)) {
	throw new Error('Related resources must keep its heading and grid as direct siblings for shared spacing');
}

const systemsMain = renderedMain('systems/index.html');
const categoryCards = [...systemsMain.matchAll(/<section class="ots-category-card"[^>]*>([\s\S]*?)<\/section>/g)];
if (categoryCards.length === 0 || categoryCards.some(([, content]) => !/^<h3\b[^>]*>[\s\S]*?<\/h3><p>/.test(content))) {
	throw new Error('System category cards must keep each title directly adjacent to its description');
}

const referenceMain = renderedMain('reference/index.html');
if (!/<div class="not-content ots-reference-primary-action"><a\b[^>]*class="[^"]*\bsl-link-button\b[^"]*"[\s\S]*?<\/a><\/div><h2>Files<\/h2>/.test(referenceMain)) {
	throw new Error('Reference landing must keep its primary action in the dedicated spacing wrapper before Files');
}

for (const [relative, rootMarker] of [
	['reference/rules/buildingtype/index.html', 'data-reference-table'],
	['commands/index.html', 'data-command-index'],
	['mapping/actions/index.html', 'data-scripting-table'],
	['reference/enums/mission/index.html', 'data-enum-table'],
]) {
	const main = renderedMain(relative);
	const root = main.match(new RegExp(`<div class="[^"]*\\bots-filter-table\\b[^"]*" ${rootMarker}>[\\s\\S]*?<p class="ots-empty"`))?.[0];
	if (!root || !/<div class="ots-reference-controls">[\s\S]*?<\/div><div class="ots-tablewrap">/.test(root)) {
		throw new Error(`${relative} must keep controls and results as direct children of the filter-table frame`);
	}
}

/* Counts, titles and credits come from the change records themselves, so a
   record added or withdrawn does not need an edit here; the count still has to
   agree with what the page rendered, including its plural. */
const changesIndex = readFileSync(resolve('dist/changes/index.html'), 'utf8');
for (const release of releaseExpectations(base)) {
	const required = [
		release.version,
		release.anchor,
		release.upgradeHref,
		`Upgrade to ${release.version}`,
		'data-change-list',
		...release.titles.map(escapeHtml),
		...release.credits.map(escapeHtml),
	];
	if (release.status === 'development') required.push('In development');
	for (const text of required) {
		if (!changesIndex.includes(text)) {
			throw new Error(`Changes index does not contain ${JSON.stringify(text)}`);
		}
	}
	// Compared as the whole element: a count reads as present inside the next
	// number up, and "1 change" sits inside "1 changes".
	const section = changesIndex.slice(changesIndex.indexOf(`id="${release.anchor}"`));
	const rendered = section.match(/<small class="ots-release-count">([^<]*)<\/small>/)?.[1];
	if (rendered !== release.count) {
		throw new Error(`Changes index counts ${release.version} as ${JSON.stringify(rendered)}; expected ${JSON.stringify(release.count)}`);
	}
}
const changesNav = changesIndex.slice(
	changesIndex.indexOf('data-manual-view-nav="changes"'),
	changesIndex.indexOf('</nav>', changesIndex.indexOf('data-manual-view-nav="changes"')),
);
for (const text of ['Changes index', 'Upgrade guide', inDevelopment.upgradeHref, releaseNavHref]) {
	if (!changesNav.includes(text)) {
		throw new Error(`Changes local navigation does not contain ${JSON.stringify(text)}`);
	}
}

const commandsNav = renderedLocalNavigation('commands/index.html', 'commands');
for (const text of ['Command index', 'Fixed controls']) {
	if (!commandsNav.includes(text)) {
		throw new Error(`Commands local navigation does not contain ${JSON.stringify(text)}`);
	}
}
if (commandsNav.includes('?audience=')) {
	throw new Error('Commands local navigation still carries audience filter deep links');
}
const usingNav = renderedLocalNavigation('using/index.html', 'using');
if (!usingNav.includes(`href="${base}/using/command-line/"`)) {
	throw new Error('Using local navigation does not link Command line options');
}

const discordInvite = 'https://opents.net/discord';
const discordPages = readdirSync(resolve('dist'), { recursive: true })
	.filter((path) => path.endsWith('.html'))
	.filter((path) => readFileSync(resolve('dist', path), 'utf8').includes(discordInvite));
if (isDemoPublication && discordPages.length > 0) {
	throw new Error('Demo publication includes the Discord link in ' + discordPages[0]);
}
if (!isDemoPublication && discordPages.length === 0) {
	throw new Error('Normal publication is missing the Discord link');
}

const documentationStatusPage = readdirSync(resolve('dist'), { recursive: true })
	.filter((path) => path.endsWith('.html'))
	.find((path) => {
		const html = readFileSync(resolve('dist', path), 'utf8');
		return html.includes('aria-label="Documentation status"') || html.includes('Explanation wanted');
	});
if (documentationStatusPage) {
	throw new Error(`Reader-facing documentation status UI or copy remains in ${documentationStatusPage}`);
}

for (const [alias, destination] of [
	['mapping/actions/41/index.html', '/mapping/actions/taction-play-anim/'],
	['mapping/events/34/index.html', '/mapping/events/tevent-near-waypoint/'],
	['mapping/missions/6/index.html', '/mapping/missions/tmission-loop/'],
]) {
	const html = readFileSync(resolve('dist', alias), 'utf8');
	if (!html.includes('Redirecting to:') || !html.includes(destination)) {
		throw new Error(`${alias} is not a compatibility redirect to ${destination}`);
	}
}
/* Both primary entry points carry the same button affordance the reference
   landing already uses, and neither falls back to a bare link. */
const landingHtml = readFileSync(resolve('dist/index.html'), 'utf8');
const landingActions = landingHtml.slice(
	landingHtml.indexOf('sl-flex actions'),
	landingHtml.indexOf('sl-markdown-content'),
);
const heroActions = [...landingActions.matchAll(/<a class="sl-link-button ([^"]*)" href="([^"]*)"/g)]
	.map(([, classes, href]) => ({
		href,
		variant: ['primary', 'secondary', 'minimal'].find((name) => classes.split(' ').includes(name)),
	}));
for (const [href, variant] of [
	[`${base}/using/`, 'primary'],
	[`${base}/reference/all/`, 'secondary'],
	[`${base}/how-to-read/`, 'secondary'],
]) {
	const action = heroActions.find((candidate) => candidate.href === href);
	if (!action) throw new Error(`Landing page does not offer ${href} as an entry point`);
	if (action.variant !== variant) {
		throw new Error(`Landing entry point ${href} renders as ${action.variant}; expected ${variant}`);
	}
}
if (heroActions.some((action) => action.variant === 'minimal')) {
	throw new Error('A landing entry point still renders without a button affordance');
}

/* The AI-assistance notice keeps its quiet wrapper at the foot of the page and
   is never promoted into the entry points above it. */
if (landingHtml.split('<p class="ots-site-note">').length !== 2) {
	throw new Error('The landing page must carry the AI-assistance notice exactly once, in .ots-site-note');
}
if (landingHtml.indexOf('<p class="ots-site-note">') < landingHtml.lastIndexOf('sl-link-card')) {
	throw new Error('The AI-assistance notice no longer sits below the landing page content');
}
if (landingActions.includes('ots-site-note')) {
	throw new Error('The AI-assistance notice was promoted into the landing entry points');
}

/* How to read the manual describes the whole manual, not the INI reference
   alone, while keeping the fragments other pages can link to. */
const howToReadMain = renderedMain('how-to-read/index.html');
for (const text of [
	'>Kinds of page</h2>', '>Finding a page</h2>', '>Reading a reference entry</h2>',
	'Feature pages', 'Guides', 'Format pages', 'Engine internals', 'Change records',
]) {
	if (!howToReadMain.includes(text)) {
		throw new Error(`How to read the manual does not contain ${JSON.stringify(text)}`);
	}
}
if (howToReadMain.includes('Key and scripting pages combine a compact specification')) {
	throw new Error('How to read the manual still opens as a reference-only page');
}
for (const id of ['specifications', 'behavior-notes', 'when-omitted', 'scopes', 'releases-and-history']) {
	if (!howToReadMain.includes(`id="${id}"`)) {
		throw new Error(`How to read the manual dropped the stable #${id} fragment`);
	}
}

/* A reader can see where they are: the sidebar marks the open page when it
   lists it, and the entry that contains it when it does not. */
const sidebarPlaceMarks = (relative, view) =>
	[...renderedLocalNavigation(relative, view).matchAll(
		/<a\b[^>]*\baria-current="(page|location)"[^>]*>\s*<span[^>]*>([^<]*)<\/span>/g,
	)].map((match) => `${match[1]}:${match[2]}`);
for (const [relative, view, expected] of [
	['keys/aa/index.html', 'reference', [`location:${UI.navigation.allKeys}`]],
	['commands/follow/index.html', 'commands', [`location:${UI.navigation.browseCommands}`]],
	['mapping/actions/taction-play-anim/index.html', 'mapping', [`location:${UI.navigation.actions}`]],
	['using/command-line/help/index.html', 'using', [`location:${UI.navigation.commandLine}`]],
	['reference/rules/buildingtype/index.html', 'reference', ['page:BuildingType']],
	['internals/locomotion/index.html', 'internals', ['page:Locomotion and piggybacking']],
]) {
	const actual = sidebarPlaceMarks(relative, view);
	if (JSON.stringify(actual) !== JSON.stringify(expected)) {
		throw new Error(`${relative} sidebar marks ${actual.join(', ') || 'nothing'}; expected ${expected.join(', ')}`);
	}
}

for (const [relative, expected] of cases) {
	const path = resolve('dist', relative);
	if (!existsSync(path)) throw new Error(`Rendered contract is missing: ${relative}`);
	const html = readFileSync(path, 'utf8');
	for (const text of expected) {
		if (!html.includes(text)) {
			throw new Error(`${relative} does not contain ${JSON.stringify(text)}. `
				+ 'These are samples of rendered output, not invariants: if the page changed '
				+ 'deliberately, update the sample rather than the page.');
		}
	}
	if (!html.includes('template=documentation_feedback.md')) {
		throw new Error(`${relative} does not include its prefilled documentation-feedback link`);
	}
}

console.log(`OK       ${cases.length} representative rendered page contracts`);
