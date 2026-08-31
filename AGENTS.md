# OpenTS repository instructions

These instructions apply to the entire repository. A more specific `AGENTS.md`
may add rules for its subtree.

## Required context

Read `CONTRIBUTING.md` before changing the project. Read `docs/BUILDING.md`
before making or repeating a build claim, and read `docs/STYLE.md` before
changing source.

For work under `manual/`, read and follow `manual/AGENTS.md`; it supplements
these instructions and routes authoring and maintenance work to the owning
manual guides.

## Project intent

OpenTS maintains both a playable engine and a platform for modding and engine
development. Internal implementation may change, but compatibility breaks
need an explicit decision and migration guidance.

The archived TibSun reconstruction and original executable are historical
evidence. Binary matching is not the acceptance criterion for active OpenTS
development.

Visual Studio 2022 Win32 Debug and Release are the supported build target.
A build result is not runtime evidence.

## Writing prose

Write documentation and project communication in plain, direct English. Use
concrete claims and familiar words. Cut restatement, stock caveats,
meta-commentary about the writing or edit, and ceremonial conclusions that
only recap the preceding text.

Keep reference and workflow documents dense and easy to scan. History,
rationale, and other narrative documents may keep the context, detail, and
transitions needed to explain why events or decisions matter. Never remove a
useful distinction, qualification, or example merely to make a document
shorter.

Treat AI-assisted prose as a draft. Verify every claim, then rewrite and trim
it before submission. The contrasts below illustrate the style; they are not
project facts or templates to copy.

- Reference prose: avoid "This option is used in order to select which output
  directory will be used." Prefer "`OutputDirectory=` selects the output
  directory."
- Narrative prose: avoid "The team adopted the library. Problems led to its
  replacement." Prefer "The team first adopted the library because it matched
  the existing API. Testing later exposed timing differences, so it was
  replaced."

## Source changes

- Use C++20 for new and substantially rewritten code, but modernize inherited
  code incrementally.
- Shape new and substantially rewritten code so the incremental migration
  toward an entity-component architecture stays possible; `docs/DIRECTION.md`
  records the direction.
- Keep mechanical changes separate from behavior and format only touched code.
- Follow local naming and layout; do not rename honest reconstruction
  placeholders without evidence.
- Classify what a change does to externally visible behavior — preserved,
  fixed, or intentionally changed — and state the evidence.
- Treat configuration, data formats, persistence, replays, networking,
  deterministic simulation, COM, ABI, and layout-sensitive structures as
  compatibility boundaries.
- Trace initialization, ownership, callers, persistence, and external effects
  before moving state or replacing a subsystem.
- Leave historical file headers and legal notices unchanged.
- Keep comments sparse and brief: one sentence on what the code cannot show,
  never the edit itself. `code/AGENTS.md` supplies the hook's comment rules.

## Documentation and validation

- Update the owning documentation for material changes to behavior,
  interfaces, configuration, builds, architecture, compatibility, or workflow.
- If no documentation changes are needed, state why the existing documentation
  remains accurate.
- Give each fact one owner and link to it instead of copying it.
- Run the narrowest relevant checks first and report exact commands,
  configurations, environments, results, and relevant checks not run.
- Use `docs/BUILDING.md` as the authority for build support. Never turn a
  configuration or build result into a runtime claim.
- Automated tests must not require proprietary game assets or original game
  executables.

## Repository safety

- Never commit game assets, original binaries, proprietary SDKs, credentials,
  personal data, IDE state, or build output.
- Preserve unrelated work in a dirty worktree.
- Commit, rewrite history, push, publish, or release only on explicit user
  request.
- Never add `Co-authored-by` trailers or AI-attribution lines.
- Use an imperative commit subject of at most 72 characters. Omit the body by
  default; add only a brief factual exception when necessary.
- Never put inventories, validation logs, pull-request summaries, or narrative
  descriptions in commit messages.
