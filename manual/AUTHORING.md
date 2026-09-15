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
is constructed or reset between reads. If state survives a read, the initializer
applies only to the first read, so trace what the engine holds at the read site
and record that concrete value. `kind: unchanged` is never used: carry-over is
default behavior the record does not document.

A full baseline re-read also counts as a reset. Type definitions survive across
scenarios, but the rules tree is re-read from the start, so a setting absent
from every rules file returns to its initializer and uses `kind: value`.
`[SpecialFlags]` has no baseline file and is read only from the scenario.
Its record states the concrete value the scenario state holds at the read. If a
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

### Trace what the engine does, not what the catalog restates

Split a page's behavioral claims before tracing any of them.

- **Trace in full** anything about what the engine does: a formula, an ordering among steps, a
  condition or a refusal, a count, a limit, a unit of measure, an interaction between two settings,
  or a claim that contradicts a sibling page.
- **Accept without tracing** what the generated catalog already establishes: a key's spelling, its
  file, its section selector, its value type, its applies-to types, and whether it is registered.
  The catalog is derived from the source and is the authority on these; re-deriving one adds
  nothing to the page.

`when_omitted` and `no_effect` are not in the second group. They are authored, and the catalog's
`default_candidate` is only a candidate.

### Frontmatter prose is prose

On an enum page most of the reader-facing text is frontmatter: `values[].meaning` renders into the
table and is the page's whole modder-facing payload. Scripting pages carry `valid_values` and
`caveats` the same way, and format pages carry `fields[].note`.

Give those fields the same claim-by-claim trace as the body. A table is trusted more readily than a
paragraph, so an error there costs more.

### Reachability, not existence

A code branch proves the engine has a behavior; it does not prove a reader can reach it. Document
paths a reader can reach. Leftovers from removed or unfinished features — an edit mode with no way
in, an error handler whose body is empty — are not behavior however clearly the code states them.

OpenTS ships no scenario editor. The tree keeps remnants of one, and none of them is reachable in
play, so no page describes what happens in it.

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

### Have the page read by someone who has not read the source

An author cannot judge whether a page explains itself: tracing a source path destroys the ability
to tell what the page fails to carry, and a page that reads correctly to the person who just
verified it may still leave a reader stuck.

Before handoff, have the page read by someone with the rendered page and no repository access, and
ask what they would write and where. That question is the gate. A reader who cannot answer it has
found a real gap, not a preference.

Questions about what they could not picture, or expected and did not get, are suggestions rather
than failures — some of what they ask for is out of scope or owned by another page, and satisfying
it would duplicate what that page renders. Record those with the reason they were refused.

Feed the reader the page as it renders, not the Markdown: the specification block, the omission
and no-effect records, the source-file list, the target text of each link, and any backlink the
template merges in from another page. Withholding those produces complaints about information the
page already carries.

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

Write for someone comparing released builds. An addition states only the new
behavior: what the reader can now do. Everything else is a pair: what the
behavior was, and what it is now. Either way, two or three sentences, then stop.

Order a pair however the record reads best. Lead with the old behavior when the
fault is what the reader recognizes, and with the new one when the record
answers "what can I do now."

- Give the reader the difference, not the mechanism that produced it. Name the
  faulty result rather than the routine, buffer, or call that caused it.
- Remove background, restatement, why the change matters, and details that do
  not change a reader's decision. In an addition, the gap the feature fills is
  background: do not open by saying what was impossible before. Do not list what
  stayed the same unless a reader would otherwise assume it changed.
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
  is for, such as a prior implementation the change follows. One sentence is
  the limit.

A change wide enough to need more than three sentences states each visible
change the same way and stops at one paragraph each. Length comes from the
number of visible changes, never from explaining one of them.

A record that names a key the reader sets names its file and section and states in prose what the
key does, including which direction a value moves the behavior. The accepted values and the default
belong on the key page, which renders them from structured fields; the record reaches them through
its `targets` list. A record with an empty `targets` list carries no route to them.

State a visible symptom where there is one: what the reader will see, or what they must change.
A record describing only an internal state transition leaves a reader unable to tell whether it
affects them. Where a change genuinely has no visible effect, say what the reader can now do
instead.

## Handoff

Report the exact commands run and their results. A schema check, site build,
preview, or runtime observation proves only what it tested. Do not commit, push,
deploy, or publish without an explicit request.
