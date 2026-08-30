/*
 * Public reference taxonomy. Generated evidence is grouped into useful views,
 * while every setting retains one canonical /keys/<route-id>/ page.
 */

import {
	isImageSection,
	isLiteralSection,
	requireSectionSelector,
	sectionSelectorKey,
	sectionSelectorLabel,
	sectionSelectorSlug,
} from './section-selector.mjs';

export const REFERENCE_PAGES = {
	rules: {
		title: 'Rules',
		files: ['rules.ini'],
		blurb: 'Object-type settings and named sections from rules.ini and firestrm.ini.',
	},
	art: {
		title: 'Art',
		files: ['art.ini'],
		blurb: 'Artwork and presentation settings grouped by effective object type.',
	},
	other: {
		title: 'Other INI files',
		files: ['sun.ini', 'theme01.ini', 'battle.ini', 'sound01.ini'],
		blurb: 'Player options, campaign catalogs, sound definitions, and music metadata grouped by source file.',
	},
};

export const OTHER_INI_FILES = [
	{
		id: 'sun',
		label: 'SUN.INI',
		title: 'Options file',
		sourceFiles: ['sun.ini'],
		purpose: 'Local player options, display and audio preferences, and multiplayer connection settings.',
		loadBehavior: 'The game reads and writes this per-install options file. Its settings use named sections.',
		groupOrder: ['options', 'video', 'audio', 'network', 'multiplayer', 'serialdefaults', 'syncbug'],
	},
	{
		id: 'campaigns',
		label: 'BATTLE*.INI',
		title: 'Campaign definitions',
		sourceFiles: ['battle.ini'],
		purpose: 'Campaign sections and their starting scenario, disc, final movie, and expansion requirements.',
		loadBehavior: 'The game loads the available BATTLE*.INI catalogs, including the Firestorm campaign catalog.',
		sectionGroup: { slug: 'campaign', title: 'Campaign sections' },
	},
	{
		id: 'sounds',
		label: 'SOUND.INI / SOUND01.INI',
		title: 'Sound effects',
		sourceFiles: ['sound01.ini'],
		purpose: 'Priority and volume settings in each sound section.',
		loadBehavior: 'The base game selects SOUND.INI. Firestorm selects SOUND01.INI instead.',
		sectionGroup: { slug: 'sounds', title: 'Sound sections' },
	},
	{
		id: 'themes',
		label: 'THEME.INI + THEME01.INI',
		title: 'Music tracks',
		sourceFiles: ['theme01.ini'],
		purpose: 'Titles, duration, side restrictions, and playback behavior in each theme section.',
		loadBehavior: 'THEME.INI loads first. Firestorm then layers THEME01.INI onto the same theme database.',
		sectionGroup: { slug: 'themes', title: 'Theme sections' },
	},
];

export function otherIniFile(sourceFile) {
	return OTHER_INI_FILES.find((file) => file.sourceFiles.includes(sourceFile));
}

export function referenceFileLabel(sourceFile) {
	return otherIniFile(sourceFile)?.label ?? sourceFile;
}

export const MAPPING_FILES = ['map file', 'map file (.mpr)', 'scenario packet (.pkt)'];

export const LEVEL_NAMES = {
	AbstractTypeClass: 'AbstractType',
	ObjectTypeClass: 'ObjectType',
	TechnoTypeClass: 'TechnoType',
};

/* Rules scopes that describe engine-wide behavior rather than an object type. */
const RULES_BEHAVIOR_SCOPES = new Set(['global rules', 'difficulty settings', 'mission behavior']);

export function applicabilitySummary(appliesTo = [], level = '') {
	const exact = [...new Set(appliesTo)];
	if (exact.length <= 4) return exact.join(', ');
	const root = LEVEL_NAMES[level];
	return root ? `${root} and derived types` : `${exact.length} applicable types`;
}

export function exceptionSummary(exceptions = []) {
	return exceptions.length <= 4
		? `Except ${exceptions.join(', ')}`
		: `Except ${exceptions.length} derived types`;
}

export function slugify(text) {
	return String(text)
		.toLowerCase()
		.replace(/[^a-z0-9]+/g, '-')
		.replace(/^-|-$/g, '');
}

/** Stable once published: a spelling change is a new key plus a tombstone. */
export function keySlug(name, entry) {
	let slug = name.toLowerCase();
	if (entry.case_collides_with?.length) {
		slug += `-${slugify(entry.scopes[0].applies_to[0] ?? 'global')}`;
	}
	return slug;
}

function finishGroup(group) {
	group.rows.sort((a, b) => a.key.localeCompare(b.key));
	return group;
}

function effectiveArtGroups(raw) {
	const groups = new Map();
	for (const [key, entry] of Object.entries(raw)) {
		for (const [scopeIndex, scope] of entry.scopes.entries()) {
			if (scope.file !== 'art.ini') continue;
			const selector = requireSectionSelector(scope.section, `INI key "${key}" scope ${scopeIndex + 1}`);
			const literal = isLiteralSection(selector);
			const applies = scope.applies_to?.length ? scope.applies_to : ['Global sections'];
			for (const type of applies) {
				const title = literal ? sectionSelectorLabel(selector) : type;
				const groupId = literal ? sectionSelectorKey(selector) : `type:${type}`;
				let group = groups.get(groupId);
				if (!group) {
					group = {
						title,
						slug: literal ? sectionSelectorSlug(selector) : slugify(title),
						literal,
						shared: 0,
						rows: [],
					};
					groups.set(groupId, group);
				}
				group.rows.push({
					key,
					keySlug: keySlug(key, entry),
					scopeIndex,
					scope,
					viaImage: isImageSection(selector),
				});
			}
		}
	}
	return [...groups.values()]
		.map(finishGroup)
		.sort((a, b) => {
			if (a.literal !== b.literal) return a.literal ? 1 : -1;
			return a.title.localeCompare(b.title);
		});
}

function hierarchyGroups(raw, page) {
	const files = REFERENCE_PAGES[page].files;
	const groups = new Map();

	for (const [key, entry] of Object.entries(raw)) {
		for (const [scopeIndex, scope] of entry.scopes.entries()) {
			if (!files.includes(scope.file)) continue;
			const selector = requireSectionSelector(scope.section, `INI key "${key}" scope ${scopeIndex + 1}`);
			const literal = isLiteralSection(selector);
			if (!scope.level) {
				throw new Error(`INI key "${key}" has no public level; run manual/tools/manage.py update`);
			}
			const level = LEVEL_NAMES[scope.level];
			const title = literal ? sectionSelectorLabel(selector) : (level ?? scope.applies_to.join(', '));
			const groupId = literal ? `${scope.file}:${sectionSelectorKey(selector)}` : `${scope.file}:${title}`;
			let group = groups.get(groupId);
			if (!group) {
				group = {
					title,
					slug: literal ? sectionSelectorSlug(selector) : slugify(title),
					file: scope.file,
					literal,
					shared: literal ? 0 : level ? 100 : scope.applies_to.length,
					subtitle: undefined,
					rows: [],
				};
				groups.set(groupId, group);
			}
			group.rows.push({
				key,
				keySlug: keySlug(key, entry),
				scopeIndex,
				scope,
				viaImage: isImageSection(selector),
			});
		}
	}

	const list = [...groups.values()];
	for (const group of list) {
		finishGroup(group);
		if (group.literal) continue;
		if (page === 'rules' && RULES_BEHAVIOR_SCOPES.has(group.title)) {
			group.behavior = true;
			const sentence = group.title.charAt(0).toUpperCase() + group.title.slice(1);
			group.navTitle = sentence;
			group.displayTitle = sentence;
		}
		const union = [...new Set(group.rows.flatMap((row) => row.scope.applies_to))].sort();
		if (group.shared === 100) {
			group.shared = union.length;
			for (const row of group.rows) {
				const missing = union.filter((type) => !row.scope.applies_to.includes(type));
				if (missing.length) row.except = missing;
			}
		}
	}

	return list.sort((a, b) => {
		if (a.literal !== b.literal) return a.literal ? 1 : -1;
		if (a.shared !== b.shared) return b.shared - a.shared;
		return a.title.localeCompare(b.title);
	});
}

function contextualizeOtherGroups(groups) {
	for (const group of groups) {
		const file = otherIniFile(group.file);
		if (!file) continue;
		group.fileId = file.id;
		group.fileLabel = file.label;
		if (!group.literal && file.sectionGroup) {
			group.slug = file.sectionGroup.slug;
			group.navTitle = file.sectionGroup.title;
			group.displayTitle = `${file.sectionGroup.title} in ${file.label}`;
		} else {
			group.navTitle = group.title;
			group.displayTitle = `${group.title} in ${file.label}`;
		}
	}

	return groups.sort((a, b) => {
		const fileA = OTHER_INI_FILES.findIndex((file) => file.id === a.fileId);
		const fileB = OTHER_INI_FILES.findIndex((file) => file.id === b.fileId);
		if (fileA !== fileB) return fileA - fileB;
		const meta = OTHER_INI_FILES[fileA];
		const orderA = meta?.groupOrder?.indexOf(a.slug) ?? -1;
		const orderB = meta?.groupOrder?.indexOf(b.slug) ?? -1;
		if (orderA !== orderB) return orderA - orderB;
		return a.title.localeCompare(b.title);
	});
}

export function referenceGroups(raw, page) {
	if (!(page in REFERENCE_PAGES)) throw new Error(`Unknown reference family: ${page}`);
	if (page === 'art') return effectiveArtGroups(raw);
	const groups = hierarchyGroups(raw, page);
	return page === 'other' ? contextualizeOtherGroups(groups) : groups;
}

export function scenarioGroups(raw) {
	const groups = new Map();
	for (const [key, entry] of Object.entries(raw)) {
		for (const [scopeIndex, scope] of entry.scopes.entries()) {
			if (!MAPPING_FILES.includes(scope.file)) continue;
			const selector = requireSectionSelector(scope.section, `INI key "${key}" scope ${scopeIndex + 1}`);
			const groupId = sectionSelectorKey(selector);
			let group = groups.get(groupId);
			if (!group) {
				group = {
					title: sectionSelectorLabel(selector),
					slug: sectionSelectorSlug(selector),
					file: scope.file,
					literal: isLiteralSection(selector),
					rows: [],
				};
				groups.set(groupId, group);
			}
			group.rows.push({
				key,
				keySlug: keySlug(key, entry),
				scopeIndex,
				scope,
				viaImage: isImageSection(selector),
			});
		}
	}
	return [...groups.values()].map(finishGroup).sort((a, b) => a.title.localeCompare(b.title));
}
