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

Visual Studio 2022 Win32 and x64, each in Debug and Release, are the
supported build targets. A build result is not runtime evidence.

## Writing prose

Write documentation and project communication in plain, direct English. Lead
with the result, then its condition, then only the mechanism a reader needs to
predict an exception. Use concrete claims and familiar words. Cut restatement,
"not X but Y" contrasts, dramatic titles, stock caveats, meta-commentary about
the writing or edit, and ceremonial conclusions that only recap the preceding
text. Do not replace them with forced casualness, fragments, or a performed
voice.

These prose rules apply to repository files and durable project communication:
documentation, manual pages, change records, source comments, commit messages,
and pull requests. Source comments also follow `## Comments` in
`code/AGENTS.md`, and manual pages and change records also follow the manual
guides. The rules do not constrain chat-only execution plans or status
updates. Plans should prioritize clarity, dependencies, risks, and
verification over brevity.

In reference and workflow documents, give every sentence a fact the reader can
act on and every paragraph one job. Keep a mechanism only when it explains an
observable result, a configuration decision, or an invariant the reader relies
on. Leave out the rest, such as an explanation that a failure goes unreported
or the sequence of calls behind a result already stated. History, rationale,
and other narrative documents may keep the context, detail, and transitions
needed to explain why events or decisions matter. Length is not the measure:
lengthen a sentence when the shorter version is ambiguous, and never remove a
useful distinction, qualification, or example merely to make a document
shorter.

### Plan before drafting

Before choosing words, work out the questions the reader brings and the
verified facts that answer them. For a configurable behavior these are usually
what it does, what controls it, what limits it, and what happens in
exceptional cases; take the questions from the subject rather than imposing
that list. Draft from those facts and open with the answer. To fix an unclear
passage, redraft it from its facts: shorter sentences in the old order leave
the reader the same work.

### Keep claims true

- Check each outcome against the conditions that make it true, even when the
  sentence has no word such as "always." Follow the branches to their exits
  and look for a reachable case that contradicts the wording.
- Keep apart the cases the code separates, such as no eligible choice, a
  failed creation, a failed placement, and success. Keep the deciding
  condition beside its result.
- A claim that something survives, stays unchanged, or is protected must also
  hold through later cleanup, deletion, and reset.
- For a formula, check which values it uses, whose modifiers apply, where it
  rounds and clamps, and what happens at the boundaries. Equal multipliers do
  not cancel exactly when intermediate values are rounded. Check every worked
  example against the actual arithmetic.
- Text carried over from an older version needs the same evidence as new
  text. Where the evidence is incomplete, narrow the claim or keep its
  qualification.

### Choose words for the reader

- Use the reader's terms for what they see and set, such as an object's
  experience, its owner, a cost, or a delay. Keep exact names where the reader
  must type or search for them.
- Use concrete subjects and familiar verbs. Do not personify code or data, and
  do not use a metaphor where the exact consequence can be stated.
- Use one term for one thing throughout. Do not rotate synonyms for variety.
- Define a term the reader would otherwise have to translate, such as
  "resolved" or "handed out," or replace it with the observable action.

### Shape the text

- Give each paragraph one job from its first sentence to its last. Split a
  paragraph that mixes separate rules, and keep each condition with its
  consequence.
- Put an exception next to the rule it qualifies. A short, concrete
  qualification is better than an absolute claim followed by a distant
  warning.
- Use tables for real mappings, lists for choices or conditions, and headings
  for topics a reader will look up. Do not give every short paragraph its own
  heading or callout.
- Write a warning as an instruction: what to set or avoid, and what happens
  otherwise. Keep warnings for real traps.

### Review before handoff

Treat AI-assisted prose as a draft. Verify every claim, then rewrite and trim
it before submission. These rules bind every edit path, including files
written through scripts or shell commands; review the final diff against them
before handing off.

- Read each section's opening on its own. It should state the main behavior,
  instruction, or condition.
- Reread the whole text, including passages you did not change. Look for
  dropped prerequisites, widened claims, ambiguous pronouns, examples that no
  longer match their explanation, and neighboring statements that contradict
  each other.
- Stop when the facts read clearly. Do not tune toward a word count, a reading
  grade, or an AI-detector score.

The contrasts below illustrate the style; they are not project facts or
templates to copy.

- Reference prose: avoid "This option is used in order to select which output
  directory will be used." Prefer "`OutputDirectory=` selects the output
  directory."
- Narrative prose: avoid "The team adopted the library. Problems led to its
  replacement." Prefer "The team first adopted the library because it matched
  the existing API. Testing later exposed timing differences, so it was
  replaced."
- Condition and outcome: avoid "A failed request rejoins the queue." Prefer
  "A request that timed out rejoins the queue; one the server refused is
  dropped."
- Plain terms: avoid "The loader asks the cache, which turns stale entries
  away." Prefer "The loader reads a stale entry from disk."
- Warning: avoid "The path must resolve." Prefer "Set `OutputDirectory=` to an
  existing folder; otherwise nothing is written."

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
