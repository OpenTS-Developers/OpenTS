# Engine source instructions

These instructions apply to `code/` and supplement the repository-wide
instructions in [`../AGENTS.md`](../AGENTS.md).

## Read before changing source

- Use [`../docs/BUILDING.md`](../docs/BUILDING.md) for supported build paths.
- Use [`../docs/STYLE.md`](../docs/STYLE.md) for source conventions.
- Read the relevant implementation, callers, data paths, and tests first.

## Language and change discipline

- Use C++20 for new and substantially rewritten code. Modernize inherited code
  incrementally.
- Follow local naming, layout, and ownership unless the change introduces and
  validates a new pattern.
- Keep mechanical cleanup separate from behavior changes. Format only code
  touched by the change.
- Do not rename reconstruction placeholders such as `func_XXXXXX`,
  `field_XXX`, or `entry_XX` without evidence for the replacement name.
- Before moving state or replacing a subsystem, trace initialization,
  ownership, callers, teardown, persistence, and external effects.

## Compatibility boundaries

Game-data formats and defaults, saves and replays, network packets,
deterministic simulation, COM interfaces used by other code, and
layout-sensitive structures are compatibility boundaries.

- Establish current behavior before changing it and classify what the change
  does to it: preserved, fixed, or intentionally changed.
- Preserve representations and defaults. If an incompatibility is intentional,
  define it, test it, document it, and provide a migration path where needed.
- Use binary matching and the archived executable as historical evidence, not
  as the sole correctness criterion.

## Comments

Most code needs no comment. Add one concise sentence only when the code cannot
show an unexpected behavior, invariant, or compatibility constraint. Do not
restate the code or narrate the edit that produced it.

When a function needs documentation, describe its contract from the caller's
point of view: its result, prerequisites, side effects, ownership, failure
behavior, or compatibility requirements. Do not describe how its body works.
A clearly named private helper needs no comment.

The inherited tree mixes comment forms. Match local indentation and width, but
use the following forms:

- New prose uses `//` or a plain `/* */` block. `//----` separators are also
  allowed. Do not add Westwood-style `**` continuation lines or decoration.
- `///` is only for genuine XML documentation such as `<summary>`. Inherited
  trailing `///` prose does not set a convention; new trailing prose uses
  `//`.
- Keep accurate historical comments in their existing form and correct errors
  narrowly. Do not add a `/*** Name -- ***/` banner. If an existing function
  banner needs substantial rewriting, replace it with `///` XML documentation.
  If an ordinary Westwood comment needs substantial rewriting, replace it with
  `//` prose.
- Comment syntax does not establish authorship.

The examples below are illustrative only; they are not project facts or
templates to copy. In a diff example, removed text is what to avoid and added
text is preferred.

Remove narration that the code already supplies:

```diff
-// Refresh every entry.
for (Entry & entry : entries) {
    entry.Refresh();
}
```

A hidden invariant or compatibility constraint can justify one:

```cpp
// Preserve iteration order because callers store these positions.
for (Entry & entry : entries) {
    entry.Refresh();
}
```

A function comment states the caller-visible effect, not the procedure:

```diff
/// <summary>
-/// Walks the list and erases entries whose timers have expired.
+/// Removes expired entries and returns the number still active.
/// </summary>
std::size_t Prune_Expired_Entries();
```

A clear private helper needs no comment:

```diff
-// Rebuilds the lookup table.
 void Rebuild_Lookup_Table();
```
