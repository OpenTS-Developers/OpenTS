# Engine source agent instructions

These instructions apply to `code/` and supplement the repository-wide
instructions in [`../AGENTS.md`](../AGENTS.md).

## Read before changing source

- Use [`../docs/BUILDING.md`](../docs/BUILDING.md) for supported build paths.
- Use [`../docs/STYLE.md`](../docs/STYLE.md) for source conventions.
- Read the task-relevant implementation, callers, data paths, and tests before
  editing.

## Language and change discipline

- Target C++20 for new and substantially rewritten code. Modernize inherited
  code incrementally instead of mechanically converting the tree.
- Follow surrounding naming, layout, and ownership conventions unless the
  change deliberately establishes and validates a new local pattern.
- Keep mechanical cleanup separate from behavior changes. Format only code
  touched by the current change.
- Do not rename reconstruction placeholders such as `func_XXXXXX`,
  `field_XXX`, or `entry_XX` without evidence for the replacement name.
- Before moving state or replacing a subsystem, trace initialization,
  ownership, callers, teardown, persistence, and externally consumed effects.

## Compatibility boundaries

Treat game-data formats and defaults, saves and replays, network packets,
deterministic simulation, consumed COM interfaces, and layout-sensitive
structures as compatibility boundaries.

- Establish current behavior before changing it and classify what the change
  does to it: preserved, fixed, or intentionally changed.
- Preserve representations and defaults unless the contribution explicitly
  defines, tests, documents, and where necessary migrates the incompatibility.
- Use binary matching and the archived executable as historical evidence, not
  as the sole correctness criterion.

## Comments

Keep comments sparse; most edits need none. Comment only what the code cannot
show — an invariant, a compatibility constraint, a surprise — in one concise
sentence. Describe the code as it stands, never the edit that produced it:
`// removed the modem branch` narrates a diff, and that history belongs to
git. Preserve accurate historical comments; correct a wrong one narrowly.

The tree mixes comment forms. Match the surrounding indentation and width but
not the choice of form: inherited usage records what Westwood wrote and does
not authorize writing more of it. These rules beat the neighboring lines.

- Historical file headers are frozen verbatim, `Functions:` table included,
  even when the listed functions change (edits need explicit legal or
  attribution review).
- Historical `/*** Name -- ***/` banners stay while accurate. One that needs
  substantial rewriting becomes `///` XML documentation; never author a new
  banner.
- Westwood `/* ** */` blocks and old `//` prose keep their form through a
  narrow correction; a substantial amendment becomes plain `//` prose.
- `///` is XML documentation (`<summary>` etc.) and nothing else. Inherited
  trailing `///` prose is noise, not a convention; new trailing comments take
  `//`.
- New prose takes `//` or a plain `/* */` block (`//----` separators
  included). Never write the `**` continuation prefix or invent decoration.

Preserve SPDX, copyright, modification, and GPL Section 7 notices; comment
syntax does not establish authorship.

## Documentation, validation, and handoff

- Update the owning user or developer documentation with every material source
  change. If no update is required, state why the existing documentation
  remains accurate.
- Run the narrowest relevant checks first, followed by the supported build or
  test suite when the change warrants it.
- Report exact commands, configurations, environments, results, and relevant
  checks not run. Never turn an unverified result into a support claim.
- Behavior changes require focused evidence and corresponding documentation.
- Automated checks must not require proprietary game assets or original game
  executables.
