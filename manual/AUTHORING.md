# Authoring the OpenTS manual

The manual documents the current OpenTS tree. It serves experienced Tiberian
Sun modders, players evaluating active development, and contributors who
need source-facing engine explanations. Give each fact one clear owner and make
every behavioral claim no broader than its evidence.

Read [Style](STYLE.md) before writing public prose. Contract, release, and route
changes also require [Maintaining](MAINTAINING.md).

## Generated and authored material

`manage.py update` derives three catalogs from the current `code/` tree:

- `data/ini-keys.yaml` records typed INI reads and their source-backed scopes;
- `data/scripting.yaml` records trigger actions, trigger events, and team
  missions;
- `data/commands.yaml` records registered commands, fixed controls, and command
  line options.

Do not hand-edit these files. Generated records establish accepted spelling,
registration, file and section selectors, scope, declared value type, editor
metadata, and source locations where those fields exist. They do not by
themselves establish omission behavior, runtime effects, fallback order, or a
successful runtime observation.

Authored explanations live in `content/`; engine lifecycle records live in
`changes/`. Use a scaffold for a new file and replace every generated `TODO:`
with source-backed content. Schemas, adapters, aliases, tombstones, and
exclusion manifests are maintainer contracts rather than ordinary prose.

## Choose the fact owner

| Page type | Owns |
| --- | --- |
| Key | Parsing, resolution, omission, invalid values, and registration effects for one INI assignment |
| Enum | One finite domain fixed by the engine |
| Scripting entity | One trigger action, trigger event, or team script mission |
| System | Runtime rules, phases, predicates, formulas, interactions, and outcomes |
| Format | Registration, structure, loading, search behavior, and record shape |
| Guide | A procedure or troubleshooting path |
| Using OpenTS | Setup, configuration, compatibility, migration, or troubleshooting for the current OpenTS version |
| Command | One registered hotkey command, fixed control, or command line option |
| Internal | Contributor-facing architecture, state, ownership, and invariants |

A feature can require several page types. Link between them instead of copying
a format's fields into a system page, a system's algorithm into a key page, or
an assignment's behavior into a guide. Dynamic registries such as houses,
weapons, sounds, and user-defined object types are formats or registry
references, not enums.

A key page must answer what its assignment does on its own, before it links
anywhere. A reader who arrives from a search result or from `rules.ini` is
asking one question, and sending them to a fifteen-minute system article to
learn what one setting changes is a failure of the key page, not a service to
them. Single ownership governs the derivation, the tables, the decision order,
and the defect explanations — not the plain statement of effect, which the key
page always carries in full. When the effect genuinely cannot be stated without
the surrounding mechanic, that is a sign the material belongs on a system page
and the key page should carry the outcome and link to the reasoning.

A key section that has grown into an explanation of the mechanic around it has
outgrown the page. Move it to a system page, or promote it to one, and leave
the key page with the effect and the link.

Gameplay mechanics remain under `content/systems/`. A page names a category,
and the category settles which group it appears under:

| Group | Categories |
| --- | --- |
| `combat` | `combat-targeting`, `weapons-projectiles`, `superweapons-special` |
| `forces-economy` | `units-movement`, `buildings-economy` |
| `scenarios-ai` | `ai-teams`, `maps-scenarios` |
| `interface-runtime` | `interface-controls`, `rendering-presentation`, `audio-speech`, `multiplayer-networking`, `tools-diagnostics` |

Choose the category by what the page's mechanic does in the game, not by the
part of the engine that implements it: a structure's animation slots are a
building subject, and a particle system that carries damage is a weapons
subject. A category no page carries yet is left out of the navigation rather
than rendered empty.

Guide categories are `setup`, `configuration`, `files-formats`,
`compatibility-migration`, and `troubleshooting`. Using OpenTS categories are
`getting-started`, `configuration`, `compatibility-migration`, and
`troubleshooting`. Internal categories are `architecture`,
`simulation-systems`, `data-scripting`, `rendering-media`, and
`networking-persistence`.

Format kinds are `syntax`, `file`, `registry`, `record`, and `binary`. Choose
the kind that describes the public structure, not the C++ class that happens to
load it. The kind also groups the format index and the Formats navigation, so a
page reaches its readers under the heading its kind names.

## Evidence

Use the strongest available evidence for each claim:

| Evidence | Establishes | Does not establish by itself |
| --- | --- | --- |
| Generated catalog | Extracted spelling, selector, scope, type, registration, and source facts | Omission behavior or runtime result |
| Current source trace | What the inspected code path reads and does | That the result occurred in a running build |
| Runtime test | What occurred in the stated build and scenario | Untested inputs, platforms, or adjacent paths |
| Historical material | Prior behavior, provenance, or comparison | Current OpenTS behavior |

Trace concrete readers, callers, initialization or registration order, and the
first consequential use. Derive applicability from loader calls rather than
inheritance alone. Distinguish missing, empty, unknown, and unresolved inputs
when the code treats them differently. If generated scope data contradicts the
current call path, correct the extractor or its adjudication instead of writing
around the conflict.

Two build configurations are supported, and both are documented. A page must
say which one a behavior belongs to wherever they differ. The project defines
no diagnostic symbol of its own; the compiler supplies `_DEBUG` for the Debug
configuration alone, so a region guarded on it is live in one supported build
and absent from the other. Never present such a region as ordinary behavior.
Developer mode and diagnostics owns that split, and the generated command
records carry the build each command, fixed control and launch option
belongs to — take it from there rather than restating it.

For keys, `when_omitted` states the effective value or behavior when no input
sets the assignment — the value as if the reader had written it. Record `kind:
value` only when the read passes the current value through as its own default
and nothing later overwrites it. `when_omitted` and `no_effect` are
independent: an inert setting may still parse and store a value.

A constructor initializer is that value only when the structure holding it is
constructed, or otherwise reset, between the previous read and this one. Where
the structure outlives the read — a global built once at startup, a settings
block reused across scenarios — omitting the assignment leaves whatever the
last input put there, which is the initializer only on the first pass. Record
`kind: unchanged` in that case and say what it retains. Trace the reset, do not
assume one: two structures that look alike in the constructor can differ
entirely in whether anything clears them.

A re-read baseline counts as a reset. Ask not only whether the structure
survives, but whether some file is read over it in full before the input in
question. Type definitions survive every scenario, yet the rules tree is read
across them from the start each time, so a setting no rules file declares comes
back to its initializer and belongs under `kind: value`. A scenario block such
as `[SpecialFlags]` has no baseline file at all — it is read from the scenario
and nowhere else — so the previous mission's value simply stands, and that is
the case `unchanged` exists for. Where a baseline restores most inputs but not
all, the residue is a property of how the files layer, and it belongs to the
page that owns layering rather than to every record the layering touches.

When evidence stops short, narrow or omit the claim. State runtime observation
only when it was performed, and identify the build, minimum setup, and observed
result when that distinction matters. Do not add stock evidence disclaimers to
otherwise established prose.

Authored `source_files` must be repository-relative paths that exist. They make
revision-aware source links possible; they do not replace review of the prose.

Cite source by name, not by position. When a brief, review, or handoff points at
a code path, name the function, member, or enumerator, and treat any line number
as a hint beside it. A name survives edits above it, and a wrong one is obvious
to the next reader; a bare line number rots silently and sends that reader to
the wrong place. This governs working traffic between contributors. Public prose
carries no source locations at all.

AI tools can help draft, restructure, and review manual content. Use them when
helpful, but verify every claim against the current source or an identified
runtime observation, apply this guide and [Style](STYLE.md), and review the
final page. AI output is not evidence.

## Relationships and structured fields

Use System `keys` and Guide `uses_keys` for key-only navigation. Use typed
`related` references for keys, scripting entities, enums, formats, systems,
commands, guides, Using OpenTS pages, and internals; only key references may
carry a scope. Validation resolves the target and supplies reverse links.

Let structured fields render their facts. Do not maintain a second prose copy
of generated command metadata, format fields, accepted-key tables,
`when_omitted`, `no_effect`, scripting value lists, or relationship lists.

## Workflow

1. Run `python manual/tools/manage.py update` so the generated catalogs match
   the current source.
2. Choose the page type and inspect its generated record, relevant authored
   pages, current source path, and schema.
3. For a new file, run the matching command shown by
   `python manual/tools/manage.py scaffold --help`. Multi-scope keys require an
   explicit scope; enum scaffolds require a source adapter; command IDs are
   case-sensitive and must already exist in the generated catalog.
4. Author only the facts owned by the page. Follow [Style](STYLE.md).
5. Run `update` again when engine-facing catalogs may have changed. Use
   `serve` for representative desktop and narrow-layout review when presentation
   changed.
6. Run `python manual/tools/manage.py check` before handoff and inspect the diff
   for unintended generated churn, route changes, temporary files, and build
   output.

Concurrent `update` runs replace the same generated files atomically and can
collide. Retry a failed generation after the other run finishes; do not merge or
hand-edit the catalogs.

## Lifecycle records

Create a record directly under `changes/` for a genuine OpenTS engine addition,
deliberate behavior change, deprecation, or removal. Valid target types are
`key`, `action`, `event`, `mission`, `format`, `enum`, `system`, and `command`;
valid effects are `added`, `changed`, `deprecated`, and `removed`.

Do not create lifecycle records for baseline documentation, prose revisions,
source-location fixes, extraction corrections, generated-catalog migrations,
or other documentation-only work. A scripting index shift is an engine change
because the index is serialized. Adding documentation for an existing enum is
documentation work; changing that enum's accepted values or representation is
an engine change.

A breaking record requires `breaking: true` and a non-empty ordered `migration`
list. Non-breaking records must not carry migration steps. A removed entity
requires a matching tombstone; removing one scope of a still-active key requires
a scoped removal target but no tombstone for the parent.

Every record carries an author: `credit` names at least one person, followed by
anyone else the change credits. The published change page renders the list.

New records target the current development release. Released lifecycle data is
immutable, as is any change ID present in the base a check compares against;
records still local to a branch may be reorganized until it merges, verified
with `--base-ref` against that branch's own base. Release and route maintenance
are described in [Maintaining](MAINTAINING.md).

### What a record says

A record is read by someone on a released build asking what changed for them.
Give the visible change and its compatibility impact as briefly as it can be
said, then stop. Two or three sentences carry most records, and length is the
sign that the detail belongs on the page that owns it.

- Be brief before being complete. Cut any sentence a reader would not miss;
  background, qualification, and restatement are the usual surplus.
- State the outcome, not the machinery: what a reader can now do, see, or must
  migrate.
- Leave out what carries no consequence a reader can act on, such as an
  internal field, a refactor, or a structure a format gained.
- Never describe the development cycle. Interoperability between unreleased
  builds, work that landed in stages, and what an earlier draft said are not
  part of the record.
- Write one record per visible change, not per commit or pull request. While a
  release is in development, fold a new record into the existing one whenever
  both describe one change to a reader.
- Keep a record true to the code as it stands. A change that supersedes an
  unreleased record rewrites it rather than adding a second.
- Say in one sentence what a credit is for when the list alone would not show
  it, as when a change follows a prior implementation.

## Handoff

Report exactly which commands ran and their results. A successful schema check,
site build, preview, or runtime observation proves only what that action tested.
Do not commit, push, deploy, or publish unless the task explicitly requests it.
