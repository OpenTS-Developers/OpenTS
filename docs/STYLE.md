# Style

OpenTS follows the Westwood conventions found in Electronic Arts' source
releases and the inherited tree. This keeps old and new code consistent and
makes historical comparisons easier. Follow the surrounding file unless this
guide or a compatibility requirement says otherwise.

Use C++20 for new and substantially rewritten code. Modernize language,
ownership, and structure incrementally, without broad formatting passes.

See [CONTRIBUTING.md](../CONTRIBUTING.md) for contribution structure,
documentation, and validation.

## Editing discipline

- Format only touched code. Keep broad formatting and renaming separate from
  behavior changes.
- Preserve local layout and naming in inherited code.
- Keep reconstruction placeholders such as `func_XXXXXX`, `field_XXX`, and
  `entry_XX` until evidence supports a precise name.

Use the repository `.clang-format` file for mechanical formatting; it does not
reflow comments. Do not run it across unrelated source.

## Language and ownership

Prefer RAII, standard-library ownership types, explicit initialization,
`nullptr`, `override`, scoped enumerations, and compiler-checked interfaces.

Before changing types or ownership at a serialized, network, COM, ABI,
deterministic, or hardware-facing boundary, establish its representation and
consumer requirements. Use fixed-width types when the representation requires
them.

## Formatting

- Use tabs for indentation, displayed at four columns, and spaces for
  alignment.
- Keep established brace placement and local spacing.
- Separate consecutive function definitions with two blank lines.
- Center pointer and reference declarators: `TechnoClass * target`,
  `SaveStreamClass & stream`.
- Write `const` to the right of what it qualifies: `char const * name`,
  `CellClass const & cell`, `int Fetch_ID(void) const`.
- Put `code/always.h` first in implementation files that use the inherited
  precompiled-header convention.
- Group project headers before system and standard-library headers.

## Naming

- Classes use PascalCase and commonly end in `Class` or `TypeClass`.
- Functions use the established `Pascal_With_Underscores` form.
- Constants and enumerators use uppercase names with appropriate subsystem
  prefixes.
- Do not introduce `m_` prefixes into inherited classes.
- Definitions and enumerations normally live in their own `.hh` headers. Keep
  new `.hh` files free of static initialization and avoid nontrivial inline code.
- Globals and static initializations live in the underscore-prefixed init
  files. [Rationale](RATIONALE.md) records why both conventions exist.
- Preserve external names unless the change versions or migrates the
  interface.

## Comments and notices

Comment only what the code does not make clear, such as unexpected behavior,
an invariant, or a compatibility constraint. A function comment describes its
effect for the caller, not its implementation. A clearly named private helper
needs no comment. Do not restate the code or narrate the edit that produced it.

Use `//` or a plain `/* */` block for new prose. Reserve `///` for genuine XML
documentation; inherited trailing `///` prose is not a convention to follow.
Do not add Westwood-style `**` decoration.

Keep accurate historical comments and correct inaccurate ones narrowly. A
function banner that needs substantial rewriting becomes `///` XML
documentation. An ordinary Westwood comment that needs substantial rewriting
becomes `//` prose. Comment syntax does not establish authorship. Keep
historical file headers and legal notices unchanged, including an unmaintained
`Functions:` table.
