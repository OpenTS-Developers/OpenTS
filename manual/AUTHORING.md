# Authoring the OpenTS manual

The manual describes the current OpenTS tree for experienced Tiberian Sun
modders, players following development, and contributors who need engine
details. Each fact has one owner, and every behavioral claim must stay within
its evidence.

Read [Style](STYLE.md) before writing public prose. Also read
[Maintaining](MAINTAINING.md) before changing contracts, releases, or routes.

## Generated and authored material

`manage.py update` builds three catalogs from the current `code/` tree:

- `data/ini-keys.yaml` records typed INI reads and their source-backed scopes;
- `data/scripting.yaml` records trigger actions, trigger events, and team
  missions;
- `data/commands.yaml` records registered commands, fixed controls, and command
  line options.

Do not edit these files by hand. Their records establish accepted spelling,
registration, file and section selectors, scope, declared value type, editor
metadata, and source locations where available. They do not establish omission
behavior, runtime effects, fallback order, or an observed runtime result.

Explanations belong in `content/`; engine lifecycle records belong in
`changes/`. Scaffold new files and replace every `TODO:` with source-backed
content. Schemas, adapters, aliases, tombstones, and exclusion manifests are
maintainer contracts, not ordinary prose.

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

A feature may need several page types. Link them instead of copying a format's
fields into a system page, a system's algorithm into a key page, or an
assignment's behavior into a guide. Dynamic registries such as houses, weapons,
sounds, and user-defined object types are formats or registry references, not
enums.

Every key page must state the assignment's effect in full before linking to a
longer explanation. The system page owns the derivation, tables, decision order,
and defect details; the key page keeps the outcome. Move or promote any key
section that has grown into an explanation of the surrounding mechanic, then
leave a link on the key page.

Gameplay mechanics belong under `content/systems/`. Their categories determine
their navigation group:

| Group | Categories |
| --- | --- |
| `combat` | `combat-targeting`, `weapons-projectiles`, `superweapons-special` |
| `forces-economy` | `units-movement`, `buildings-economy` |
| `scenarios-ai` | `ai-teams`, `maps-scenarios` |
| `interface-runtime` | `interface-controls`, `rendering-presentation`, `audio-speech`, `multiplayer-networking`, `tools-diagnostics` |

Choose a category by what the mechanic does in the game, not by its
implementation. A structure's animation slots are a building subject; a
damage-carrying particle system is a weapons subject. Categories with no pages
stay out of the navigation.

Guide categories are `setup`, `configuration`, `files-formats`,
`compatibility-migration`, and `troubleshooting`. Using OpenTS categories are
`getting-started`, `configuration`, `compatibility-migration`, and
`troubleshooting`. Internal categories are `architecture`,
`simulation-systems`, `data-scripting`, `rendering-media`, and
`networking-persistence`.

Format kinds are `syntax`, `file`, `registry`, `record`, and `binary`. Choose
the kind that describes the public structure, not the C++ loader. The kind also
sets the page's group in the format index and navigation.

## Evidence

Use the strongest evidence available for each claim:

| Evidence | Establishes | Does not establish by itself |
| --- | --- | --- |
| Generated catalog | Extracted spelling, selector, scope, type, registration, and source facts | Omission behavior or runtime result |
| Current source trace | What the inspected code path reads and does | That the result occurred in a running build |
| Runtime test | What occurred in the stated build and scenario | Untested inputs, platforms, or adjacent paths |
| Historical material | Prior behavior, provenance, or comparison | Current OpenTS behavior |

Trace concrete readers, callers, initialization or registration order, and the
first consequential use. Derive applicability from loader calls, not
inheritance alone. Distinguish missing, empty, unknown, and unresolved inputs
when the code does. If a generated scope conflicts with the current call path,
fix the extractor or its adjudication instead of writing around it.

Pages must name the configuration when behavior differs between the two
supported builds. OpenTS defines no diagnostic symbol of its own; the compiler
defines `_DEBUG` only for Debug. Code guarded by it is therefore Debug-only,
not general behavior. The Developer mode and diagnostics page owns this split,
and generated command records state which build contains each command, fixed
control, and launch option. Do not duplicate that metadata.

For keys, `when_omitted` states the effective value or behavior when no input
sets the assignment. For a fixed default, state the value as though the reader
had supplied it. Use `kind: value` only when the read passes the current value
through as its default and nothing later overwrites it. `when_omitted` and
`no_effect` are independent; a setting can parse and store a value without
affecting behavior.

A constructor initializer is the omitted value only if the containing structure
is constructed or reset between reads. If state survives a read, omission keeps
the previous input, and the initializer applies only to the first read. Use
`kind: unchanged` and state what remains. Trace the reset instead of inferring
it from a constructor.

A full baseline re-read also counts as a reset. Type definitions survive across
scenarios, but the rules tree is re-read from the start, so a setting absent
from every rules file returns to its initializer and uses `kind: value`.
`[SpecialFlags]` has no baseline file and is read only from the scenario.
Omission uses `kind: unchanged` and keeps the previous mission's value. If a
baseline resets only some inputs, explain that layering once on the page that
owns it rather than on every affected record.

Narrow or omit claims that the evidence cannot support. Report a runtime result
only after observing it, with the build, minimum setup, and result when those
details matter. Do not add stock evidence disclaimers to established prose.

Every authored `source_files` entry must be a repository-relative path that
exists. These entries support revision-aware source links but do not replace
review of the prose.

In briefs, reviews, and handoffs, cite a function, member, or enumerator by name.
A line number may accompany it as a hint but must not stand alone. Public prose
contains no source locations.

Contributors own AI-assisted work. Verify it to the same evidence standard,
apply this guide and [Style](STYLE.md), and review the result. AI output is not
evidence.

## Relationships and structured fields

Use System `keys` and Guide `uses_keys` for key-only navigation. Use typed
`related` references for keys, scripting entities, enums, formats, systems,
commands, guides, Using OpenTS pages, and internals. Only key references may
carry a scope. Validation resolves targets and adds reverse links.

Do not repeat facts rendered from structured fields, including generated
command metadata, format fields, accepted-key tables, `when_omitted`,
`no_effect`, scripting value lists, and relationship lists.

## Workflow

1. Run `python manual/tools/manage.py update` so the catalogs match the current
   source.
2. Choose the page type and inspect its generated record, related authored
   pages, current source path, and schema.
3. For a new file, use the matching command from
   `python manual/tools/manage.py scaffold --help`. Multi-scope keys require an
   explicit scope; enum scaffolds require a source adapter; command IDs are
   case-sensitive and must already exist in the generated catalog.
4. Write only the facts owned by the page and follow [Style](STYLE.md).
5. Run `update` again if engine-facing catalogs may have changed. If presentation
   changed, use `serve` to inspect representative desktop and narrow layouts.
6. Before handoff, run `python manual/tools/manage.py check` and inspect the diff
   for generated churn, route changes, temporary files, and build output.

Concurrent `update` runs replace the same files atomically and can collide. If
generation fails during another run, retry afterward; do not merge or hand-edit
the catalogs.

## Lifecycle records

Create a record directly under `changes/` for a genuine OpenTS engine addition,
deliberate behavior change, deprecation, or removal. Valid target types are
`key`, `action`, `event`, `mission`, `format`, `enum`, `system`, and `command`.
Valid effects are `added`, `changed`, `deprecated`, and `removed`.

Do not create records for baseline documentation, prose edits, source-location
fixes, extraction corrections, generated-catalog migrations, or other
documentation-only work. A scripting index shift is an engine change because
the index is serialized. Documenting an existing enum is documentation work;
changing its accepted values or representation is an engine change.

A breaking record requires `breaking: true` and a non-empty, ordered
`migration` list. A non-breaking record must not contain migration steps. A
removed entity needs a matching tombstone. Removing one scope of an active key
needs a scoped removal target, but no tombstone for the parent.

Every record's `credit` list names its author first, followed by anyone else
credited for the change; it cannot be empty. The published change page renders
the list.

New records target the current development release. Released lifecycle data is
immutable, as is any change ID present in the base revision used by the check.
Records still local to a branch may be reorganized until merge; verify them
with `--base-ref` against that branch's own base. See
[Maintaining](MAINTAINING.md) for release and route work.

### What a record says

Write for someone comparing released builds. State the visible change and its
compatibility effect in two or three sentences when possible, then stop.

- Remove background, restatement, and details that do not change a reader's
  decision.
- Describe the outcome: what the reader can do, will see, or must migrate. Omit
  internal fields, refactors, and structures with no visible consequence.
- Do not narrate the development cycle, work that landed in stages,
  interoperability between unreleased builds, or earlier drafts.
- Write one record per visible change, not per commit or pull request. During a
  development release, fold later work into the existing record when both
  describe the same visible change.
- Keep unreleased records true to the current code. Rewrite a record when later
  work supersedes it instead of adding another.
- If the names alone do not explain a credit, add one sentence saying what it
  is for, such as a prior implementation the change follows.

## Handoff

Report the exact commands run and their results. A schema check, site build,
preview, or runtime observation proves only what it tested. Do not commit, push,
deploy, or publish without an explicit request.
