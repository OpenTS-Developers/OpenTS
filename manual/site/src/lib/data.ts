/*
 * Generated-data boundary for the public manual. The extractor records source
 * evidence; this module removes provenance and exposes a stable modder-facing
 * contract to every route.
 */

import { compare, rcompare, valid } from 'semver';
import { commandHref, commandLabel } from './commands';
import { formatHref, formatLabel } from './formats';
import type {
	EntityReference,
	LifecycleEntityType,
	RelatedEntityType,
	KeyProseData,
	LifecycleEffect,
	WhenOmitted,
} from './content-contracts';
import {
	keySlug,
	referenceFileLabel,
	referenceGroups,
	scenarioGroups,
	slugify,
	applicabilitySummary,
	exceptionSummary,
	MAPPING_FILES,
	OTHER_INI_FILES,
	REFERENCE_PAGES,
} from './reference.mjs';
import type {
	RawReferenceEntry,
	RawReferenceGroup,
	RawReferenceScope,
	ReferencePage,
} from './reference.mjs';
import { withFixtures } from './fixtures.mjs';
import { loadDataYaml, loadEnumDocuments, loadFixtureYaml } from './raw-data';
import { requireSectionSelector, sectionSelectorSlug, type SectionSelector } from './section-selector.mjs';
export { displayWhenOmitted, formatWhenOmitted, summarizeWhenOmitted, whenOmittedForScope } from './omission.mjs';
export { effectForKey, effectForScope, noEffectForScope } from './no-effect.mjs';
export type { EffectState } from './no-effect.mjs';

export { OTHER_INI_FILES, REFERENCE_PAGES, applicabilitySummary, exceptionSummary, referenceFileLabel };
export type { EntityReference, LifecycleEntityType, RelatedEntityType, LifecycleEffect, WhenOmitted };


export interface LifecycleTarget extends EntityReference<LifecycleEntityType> {
	effect: LifecycleEffect;
}
export interface ReleaseRecord {
	version: string;
	status: 'development' | 'candidate' | 'released';
	date?: string;
}

let releaseCache: ReleaseRecord[] | null = null;

export function releases(): ReleaseRecord[] {
	if (releaseCache) return releaseCache;
	const rows = loadDataYaml<{ releases: ReleaseRecord[] }>('releases.yaml').releases;
	for (const row of rows) {
		if (!valid(row.version) || row.version.includes('+')) {
			throw new Error(`Invalid release version: ${row.version}`);
		}
	}
	releaseCache = [...rows].sort((a, b) => rcompare(a.version, b.version));
	return releaseCache;
}

export function releaseByVersion(version: string): ReleaseRecord | undefined {
	return releases().find((release) => release.version === version);
}

export function compareVersions(left: string, right: string): number {
	return compare(left, right);
}

export interface WhenOmittedDisplay {
	text: string;
	code: boolean;
	note?: string;
}

export interface KeyScope {
	applies_to: string[];
	file: string;
	section: SectionSelector;
	value_type: string;
	note?: string;
	precedence?: string;
	read_from?: { file: string; section: SectionSelector }[];
	route_id: string;
	level: string;
}

export interface KeyRecord {
	key: string;
	route_id: string;
	scopes: KeyScope[];
	case_collides_with?: string[];
}

export interface EnumValueRecord {
	constant: string;
	value: number;
	input: string;
	meaning: string;
	note?: string;
}

export interface EnumRecord {
	enum_id: string;
	slug: string;
	title: string;
	summary: string;
	representation: 'token' | 'integer';
	bindings: {
		key_value_types: string[];
		scripting_parameter_types: string[];
	};
	source_files: string[];
	values: EnumValueRecord[];
}

let enumCache: Map<string, EnumRecord> | null = null;

export function allEnums(): Map<string, EnumRecord> {
	if (enumCache) return enumCache;
	enumCache = new Map();
	const slugs = new Set<string>();
	for (const [path, value] of Object.entries(loadEnumDocuments())) {
		const record = value as EnumRecord;
		const filename = path.split('/').at(-1)?.replace(/\.md$/, '');
		if (filename !== record.slug) throw new Error(`${path}: filename must be ${record.slug}.md`);
		if (enumCache.has(record.enum_id)) throw new Error(`Duplicate enum ID: ${record.enum_id}`);
		if (slugs.has(record.slug)) throw new Error(`Duplicate enum route: ${record.slug}`);
		slugs.add(record.slug);
		enumCache.set(record.enum_id, record);
	}
	return enumCache;
}

export function enumForValueType(valueType: string): EnumRecord | undefined {
	return [...allEnums().values()].find((record) => record.bindings.key_value_types.includes(valueType));
}

export function enumForParameterType(parameterType: string): EnumRecord | undefined {
	return [...allEnums().values()].find((record) => record.bindings.scripting_parameter_types.includes(parameterType));
}
export interface TombstoneRecord {
	type: LifecycleEntityType;
	id: string;
	route: string;
	search_aliases: string[];
	replacement?: EntityReference<LifecycleEntityType>;
	summary: string;
}

type RawKey = RawReferenceEntry & { key: string };

function normalizeScope(scope: RawReferenceScope, context: string): KeyScope {
	if (!scope.level) throw new Error(`${context}: generated scope is missing public level; run manual/tools/manage.py update`);
	const appliesTo = scope.applies_to ?? [];
	const section = requireSectionSelector(scope.section, `${context} section`);
	const readFrom = scope.read_from?.map((read, index) => ({
		file: read.file,
		section: requireSectionSelector(read.section, `${context} read_from ${index + 1} section`),
	}));
	return {
		applies_to: appliesTo,
		file: scope.file,
		section,
		value_type: scope.value_type,
		note: scope.note,
		precedence: scope.precedence,
		read_from: readFrom,
		route_id: slugify(appliesTo[0] ?? sectionSelectorSlug(section) ?? 'global'),
		level: scope.level,
	};
}

let keyCache: Map<string, KeyRecord> | null = null;

export function allKeys(): Map<string, KeyRecord> {
	if (keyCache) return keyCache;
	const raw = loadDataYaml<Record<string, RawKey>>('ini-keys.yaml');
	const byRoute = new Map<string, string>();
	keyCache = new Map();

	for (const [name, entry] of Object.entries(raw)) {
		if (entry.key !== name) throw new Error(`INI catalog entry "${name}" declares key "${entry.key}"`);
		const routeId = keySlug(name, entry);
		const clash = byRoute.get(routeId);
		if (clash) {
			throw new Error(
				`INI route collision: "${name}" and "${clash}" both map to /keys/${routeId}/`,
			);
		}
		byRoute.set(routeId, name);

		const scopeIds = new Set<string>();
		const scopes = entry.scopes.map((scope, index) => {
			const normalized = normalizeScope(scope, `INI key "${name}" scope ${index + 1}`);
			let candidate = normalized.route_id;
			let suffix = 2;
			while (scopeIds.has(candidate)) candidate = `${normalized.route_id}-${suffix++}`;
			scopeIds.add(candidate);
			return { ...normalized, route_id: candidate };
		});

		keyCache.set(name, {
			key: name,
			route_id: routeId,
			scopes,
			case_collides_with: entry.case_collides_with,
		});
	}

	const collisionErrors: string[] = [];
	for (const record of keyCache.values()) {
		for (const otherName of record.case_collides_with ?? []) {
			const other = keyCache.get(otherName);
			if (!other) {
				collisionErrors.push(`INI key "${record.key}" names missing case collision "${otherName}"`);
			} else if (other.key === record.key || other.key.toLowerCase() !== record.key.toLowerCase()) {
				collisionErrors.push(`INI keys "${record.key}" and "${other.key}" are not distinct case-only spellings`);
			} else if (!(other.case_collides_with ?? []).includes(record.key)) {
				collisionErrors.push(`INI case collision "${record.key}" -> "${otherName}" is not symmetric`);
			}
		}
	}
	if (collisionErrors.length) throw new Error(`INI key validation failed:\n  ${collisionErrors.join('\n  ')}`);
	return keyCache;
}

export function requireKey(name: string, context: string): KeyRecord {
	const record = allKeys().get(name);
	if (!record) throw new Error(`${context}: unknown INI key "${name}"`);
	return record;
}

export function keyByRoute(routeId: string): KeyRecord | undefined {
	for (const record of allKeys().values()) {
		if (record.route_id === routeId) return record;
	}
	return undefined;
}

export function tombstones(): TombstoneRecord[] {
	return withFixtures(
		loadDataYaml<TombstoneRecord[]>('tombstones.yaml'),
		loadFixtureYaml<TombstoneRecord[]>('tombstone.yaml'),
	) as TombstoneRecord[];
}

export interface ReferenceRow {
	key: string;
	keyRoute: string;
	scope: KeyScope;
	except?: string[];
	viaImage: boolean;
}

export interface ReferenceGroup {
	title: string;
	slug: string;
	file?: string;
	fileId?: string;
	fileLabel?: string;
	displayTitle?: string;
	navTitle?: string;
	subtitle?: string;
	literal?: boolean;
	behavior?: boolean;
	shared?: number;
	rows: ReferenceRow[];
}

/*
 * Rows resolve against allKeys() rather than normalizing their own scope,
 * because only allKeys() suffixes the route ids of two readings that would
 * otherwise share one. Prose is matched by route id, so a row that carries an
 * unsuffixed id silently takes the first reading's omission and effect.
 */
function normalizeGroup(group: RawReferenceGroup): ReferenceGroup {
	const keys = allKeys();
	return {
		...group,
		rows: group.rows.map((row) => {
			const record = keys.get(row.key);
			const scope = record?.scopes[row.scopeIndex];
			if (!record || !scope) {
				throw new Error(
					`Reference group ${group.slug} names reading ${row.scopeIndex + 1} of INI key "${row.key}", which the catalog does not have`,
				);
			}
			return {
				key: row.key,
				keyRoute: record.route_id,
				scope,
				except: row.except,
				viaImage: Boolean(row.viaImage),
			};
		}),
	};
}

export function referenceSections(page: ReferencePage): ReferenceGroup[] {
	return referenceGroups(loadDataYaml<Record<string, RawKey>>('ini-keys.yaml'), page).map(normalizeGroup);
}

export function scenarioSections(): ReferenceGroup[] {
	return scenarioGroups(loadDataYaml<Record<string, RawKey>>('ini-keys.yaml')).map(normalizeGroup);
}

export function referenceRowsForFile(file: string): ReferenceRow[] {
	const rows: ReferenceRow[] = [];
	for (const record of allKeys().values()) {
		for (const scope of record.scopes) {
			if (scope.file !== file) continue;
			rows.push({
				key: record.key,
				keyRoute: record.route_id,
				scope,
				viaImage: scope.section.kind === 'image',
			});
		}
	}
	return rows.sort((left, right) =>
		left.key.localeCompare(right.key) || left.scope.route_id.localeCompare(right.scope.route_id),
	);
}

export function referenceFamily(scope: KeyScope): string {
	if (scope.file === 'map seed file') return 'map-seeds';
	if (scope.file === 'theater control file') return 'theater-controls';
	for (const [family, meta] of Object.entries(REFERENCE_PAGES)) {
		if (meta.files.includes(scope.file)) return family;
	}
	if (MAPPING_FILES.includes(scope.file) || scope.file === 'ai.ini or map file') return 'mapping';
	return 'other';
}

export interface TriggerParameter {
	name: string;
	type: string;
}

interface RawTriggerRecord {
	id: string;
	index: number;
	editor_name?: string | null;
	editor_description?: string | null;
	need: string;
	parameters: TriggerParameter[];
	ini_example?: { section: '[Actions]' | '[Events]'; line: string };
	attaches_to?: string | null;
	[key: string]: unknown;
}

interface RawScriptingData {
	trigger_actions: RawTriggerRecord[];
	trigger_action_orphan_texts: unknown[];
	trigger_events: RawTriggerRecord[];
	team_missions: RawTriggerRecord[];
}

export interface TriggerRecord extends RawTriggerRecord {
	kind: 'actions' | 'events' | 'missions';
	route_id: string;
}

interface TriggerData {
	trigger_actions: TriggerRecord[];
	trigger_action_orphan_texts: unknown[];
	trigger_events: TriggerRecord[];
	team_missions: TriggerRecord[];
}

let triggerCache: TriggerData | null = null;

export function triggerData(): TriggerData {
	if (triggerCache) return triggerCache;
	const raw = loadDataYaml<RawScriptingData>('scripting.yaml');
	const normalize = (rows: RawTriggerRecord[], kind: TriggerRecord['kind']): TriggerRecord[] =>
		rows.map((row) => ({ ...row, kind, route_id: slugify(row.id) }));
	triggerCache = {
		trigger_actions: normalize(raw.trigger_actions, 'actions'),
		trigger_action_orphan_texts: raw.trigger_action_orphan_texts ?? [],
		trigger_events: normalize(raw.trigger_events, 'events'),
		team_missions: normalize(raw.team_missions, 'missions'),
	};
	return triggerCache;
}

export function triggersByKind(kind: string): TriggerRecord[] {
	const data = triggerData();
	if (kind === 'actions') return data.trigger_actions;
	if (kind === 'events') return data.trigger_events;
	if (kind === 'missions') return data.team_missions;
	return [];
}

export interface ScriptingRouteAlias {
	route_id: string;
	target_id: string;
}

export function scriptingRouteAliases(kind: string): ScriptingRouteAlias[] {
	const data = loadDataYaml<Record<string, Record<string, string>>>('scripting-route-aliases.yaml');
	return Object.entries(data[kind] ?? {}).map(([route_id, target_id]) => ({
		route_id,
		target_id,
	}));
}

export function triggerById(type: LifecycleEntityType, id: string): TriggerRecord | undefined {
	const kind = type === 'action' ? 'actions' : type === 'event' ? 'events' : type === 'mission' ? 'missions' : '';
	return triggersByKind(kind).find((record) => record.id === id);
}

export function tombstoneFor(reference: EntityReference): TombstoneRecord | undefined {
	return tombstones().find((record) => record.type === reference.type && record.id === reference.id);
}

export function entityHref(reference: EntityReference): string | undefined {
	if (reference.type === 'key') {
		const key = allKeys().get(reference.id);
		if (key) {
			const fragment = reference.scope ? `#scope-${reference.scope}` : '';
			return `/keys/${key.route_id}/${fragment}`;
		}
	} else if (reference.type === 'format') {
		return formatHref(reference.id);
	} else if (reference.type === 'command') {
		return commandHref(reference.id);
	} else if (reference.type === 'enum') {
		const record = allEnums().get(reference.id);
		if (record) return `/reference/enums/${record.slug}/`;
	} else if (reference.type === 'system') {
		return `/systems/${reference.id}/`;
	} else if (reference.type === 'guide') {
		return `/guides/${reference.id}/`;
	} else if (reference.type === 'using') {
		return `/using/${reference.id}/`;
	} else if (reference.type === 'internal') {
		return `/internals/${reference.id}/`;
	} else if (reference.type === 'action' || reference.type === 'event' || reference.type === 'mission') {
		const record = triggerById(reference.type, reference.id);
		if (record) return `/mapping/${record.kind}/${record.route_id}/`;
	}
	return tombstoneFor(reference)?.route;
}

export function requireEntityHref(reference: EntityReference, context: string): string {
	const href = entityHref(reference);
	if (!href) throw new Error(`${context}: unresolved ${reference.type} reference "${reference.id}"`);
	return href;
}

export function entityLabel(reference: EntityReference): string {
	if (reference.type === 'key') {
		return reference.scope ? `${reference.id} [${reference.scope}]` : reference.id;
	}
	if (reference.type === 'format') return formatLabel(reference.id) ?? reference.id;
	if (reference.type === 'command') return commandLabel(reference.id);
	if (reference.type === 'enum') return allEnums().get(reference.id)?.title ?? reference.id;
	if (reference.type === 'action' || reference.type === 'event' || reference.type === 'mission') {
		const record = triggerById(reference.type, reference.id);
		return record?.editor_name ?? reference.id;
	}
	return reference.id;
}

export interface ProseEntry {
	id: string;
	data: KeyProseData;
}

interface ProseReferenceIds {
	guides: ReadonlySet<string>;
	systems: ReadonlySet<string>;
}

export function validateProse(entries: ProseEntry[], references: ProseReferenceIds): void {
	const keys = allKeys();
	const errors: string[] = [];
	for (const entry of entries) {
		const record = keys.get(entry.data.key);
		if (!record) {
			errors.push(`${entry.id}: key "${entry.data.key}" is not parsed by OpenTS`);
			continue;
		}
		const expectedId = entry.data.scope ? `${record.route_id}--${entry.data.scope}` : record.route_id;
		if (entry.id !== expectedId) {
			errors.push(`${entry.id}: filename must be ${expectedId}.md for key "${entry.data.key}"`);
		}
		if (entry.data.scope && !record.scopes.some((scope) => scope.route_id === entry.data.scope)) {
			errors.push(
				`${entry.id}: key "${entry.data.key}" has no scope "${entry.data.scope}" ` +
				`(scopes: ${record.scopes.map((scope) => scope.route_id).join(', ')})`,
			);
		}
		if (entry.data.when_omitted && !entry.data.scope && record.scopes.length !== 1) {
			errors.push(
				entry.id + ': key "' + entry.data.key + '" has ' + record.scopes.length +
				' scopes; when_omitted must be attached to a scoped record',
			);
		}
		for (const ref of entry.data.see_also) {
			if (keys.has(ref)) continue;
			if (ref.startsWith('guide:') && references.guides.has(ref.slice(6))) continue;
			if (ref.startsWith('system:') && references.systems.has(ref.slice(7))) continue;
			errors.push(`${entry.id}: see_also target "${ref}" is not a known key, guide, or system`);
		}
	}
	if (errors.length) throw new Error('Prose validation failed:\n  ' + errors.join('\n  '));
}
