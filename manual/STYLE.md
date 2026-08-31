# Manual style

Write for readers making configuration or maintenance decisions. Keep prose
short, literal, and focused on behavior that catalogs and frontmatter do not
already show.

## Audience and voice

Modder-facing pages may assume familiarity with Tiberian Sun INI syntax,
ObjectType IDs, and FinalSun terminology. Define OpenTS-specific behavior and
invariants. Internals pages may assume C++ and repository knowledge, but must
still define OpenTS ownership and compatibility boundaries.

Use terms familiar to the Tiberian Sun modding community. Add the exact engine
identifier where it helps. Preserve the spelling and case of keys, IDs, enum
tokens, filenames, command strings, and C++ symbols. Use American English and
sentence case for headings and labels.

Prefer plain terms to C++ class names. Write vehicle, infantry, aircraft, and
structure rather than `UnitClass`, `InfantryClass`, `AircraftClass`, and
`BuildingClass`. If a broad term is ambiguous, list the kinds it covers;
"unit" alone is ambiguous in this game's vocabulary. Define a term on the page
unless it recurs across many pages and has no short synonym, in which case link
the [glossary](site/src/content/docs/glossary.md).

Use present-tense, declarative sentences. Lead with the result, then give its
condition. Replace vague words such as "safe," "normal," "available," and
"works" with the exact condition or result. Avoid marketing, personification,
and editorial status.

Describe the engine and its inputs, not how the manual was produced. Do not
label public material as generated, extracted, authored, or contract-backed.
The site may carry one brief notice about AI assistance and possible errors;
do not repeat it on individual pages.

## Canonical terms

| Term | Meaning |
| --- | --- |
| Section | A bracketed INI heading such as `[GAPOWR]` |
| Key | The name to the left of `=` |
| Value | The text to the right of `=` |
| Assignment | The complete `Key=Value` line |
| ObjectType ID | A rules identifier for a type definition |
| Runtime instance | An object created from a type and present in the scenario |
| Image ID | An art identifier selected by an assignment such as `Image=` |
| TypeClass | A C++ type-definition class; use only in developer-facing prose |

Keep type-definition state, runtime-instance state, and global or scenario
state distinct. Link the first useful mention of an engine-specific concept to
its main explanation when the link helps the reader.

### Ambiguous terms

Use these terms only in the stated sense. Check each use against its own code
path; do not copy wording from a nearby page or make a global replacement.

| Term | Use |
| --- | --- |
| Mover, foot object | Name the actual kinds of object. [`CloseEnough`](content/keys/closeenough.md), for example, applies to infantry, walkers, hovercraft, and driven vehicles. |
| Leaves the map | An object literally exits across the map edge. If it is destroyed, sold, or otherwise removed from play, write "taken off the map," as in [Laser fences](content/systems/laser-fences.md). |
| Full strength | A team's requested roster count, as defined in [AI triggers and team production](content/systems/ai-team-production.md). Use "undamaged" or "maximum strength" for hit points and "full brightness" for lighting. |
| Slot | Always qualify it at first use: difficulty slot, weapon slot, cameo slot, upgrade slot, Tiberium slot, or landing slot. |
| Score | Points. Use "music" or "music track" for the other sense. |
| Under way | A team has started. Use "moving" for motion. |
| Converter, drawer | Do not use these C++ names as reader terms. Name the cell tint table, terrain tile renderer, or shape renderer meant by the code. For example, describe a cell's tint table as "the same tinted terrain palette as the ground beneath it." |

### Map regions

A scenario declares two different regions of cells. Use only these names:

| Name | Declared by | Meaning |
| --- | --- | --- |
| The playfield | `[Map] Size=` | Every cell the map has. Nothing can stand, path, or be revealed outside it. |
| The playable area | `[Map] LocalSize=` | The smaller region that the player can see, scroll to, and play in. The engine clips and insets it within the playfield; the remaining ring is the map border. |

Both regions are diamonds on the cell grid, not rectangles. The map rectangle
is a third region: the upright square of cells enclosing the playfield, used
for scanning and iteration. Define it wherever it appears.

Do not use "playable diamond," "visible area," "local map area," "local radar
area," or "scenario's declared playable area." "Playable" belongs to the
smaller region, while "visible" conflicts with shroud and fog terminology.

Never rename these regions by find and replace. Existing prose has used
"playable area" for both. Trace each claim: crates are placed in the playable
area but may be collected anywhere in the playfield; a search may be allowed
by the playable area and then scan the playfield; guarding units may survive
to the playfield while other objects are removed at the playable-area edge.

## Summaries and structure

Write `summary` as one short, literal description of the page's behavior or
purpose. It appears as the subtitle and search description, so do not repeat it
in the opening paragraph or write it as a teaser.

State a value or effect directly. For a value, write "The WeaponType in the
object type's first weapon slot," not "Names the WeaponType in the first weapon
slot." For behavior, write "Marks a map as one that shipped with the game."
Prefer wording that remains true if another scope is added later.

Use headings for structure, not decoration. Use H2 for a topic and H3 for its
phases, cases, or variants. A short page may need no headings. Do not pad a
simple entity to fit a template.

If a system page depends on an entity with its own vocabulary, open with one
clearly headed section that defines the entity, the few fields used by the
mechanic, and one short example. Readers who already know it should be able to
skip the section. Do not explain unrelated vocabulary.

Define a term of art in the sentence that first uses it. Terms such as full
strength, autocreate, stray distance, and team origin look like ordinary
English but have specific meanings.

For a runtime decision, describe only the order, accepted and rejected paths,
fallback, and result that affect use. Name a known crash, deletion,
desynchronization, collateral effect, or cleanup path instead of saying that
the operation fails.

Use a structured list for a condition with three or more terms or any nesting.
Label groups "All of," "Any of," or "None of," indent their terms, and keep the
engine's test order.

## Examples

Use the smallest concrete input that clarifies a non-obvious syntax, scope,
interaction, unit, or outcome.

- Give each INI fence a useful file title.
- Use exact keys and realistic values instead of pseudo-syntax when possible.
- Define synthetic IDs and all IDs they reference, preferably with a short
  inline comment.
- Say when an example omits required surrounding configuration.
- Explain only fields and results that the reference does not already make
  clear.
- Call a value a default, recommendation, or canonical choice only when the
  evidence supports that claim.

```ini title="rules.ini"
[GAPOWR]
BaseNormal=no
Adjacent=5
```

An example shows input shape; it does not prove that a runtime result was
observed. Examples must not require proprietary assets or original executables
to understand.

## Markdown vocabulary

Prefer ordinary Markdown:

- inline code for keys, values, IDs, filenames, sections, and expressions;
- titled fenced blocks for INI, serialized records, and necessary source;
- short lists and tables for real steps, choices, or mappings;
- Starlight asides only when the information needs to interrupt the page.

Use `note` for necessary context, `tip` for a source-supported or tested recipe,
`caution` for an easy misconfiguration or surprising exception, and `danger`
for a demonstrated crash, corruption, desynchronization, or compatibility
risk. A danger aside states the input and the outcome.

Avoid raw HTML, MDX-only widgets, one-off components, empty paragraphs, and
callouts used only for visual hierarchy. Put recurring presentation in shared
Astro components and CSS.

## Keep one source of truth

Do not repeat facts rendered from structured metadata: omission and no-effect
records, command fields, scripting value lists, format fields, accepted-key
selectors, source-file lists, or typed relationships. Prose may explain why a
fact matters, but it must not maintain another copy of the same data.

State current behavior without routine verification hedges or compatibility
promises. If the evidence is incomplete, narrow or omit the claim. Add a local
qualification only when the uncertainty changes the reader's decision.
