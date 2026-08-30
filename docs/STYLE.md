# Style

OpenTS keeps inherited source recognizable while using C++20 for new and
substantially rewritten C++ code. Follow the surrounding file unless this guide
or an explicit compatibility requirement says otherwise.

The conventions here are Westwood's own, documented in Electronic Arts' source
releases and surviving in the inherited tree. Keeping them means
reconstructed, inherited, and new code read as one codebase, and a change
stays reviewable against the historical evidence instead of fighting it.
Modernization happens incrementally in substance — language level, ownership,
subsystem structure — rather than in surface formatting sweeps.

Contribution structure, documentation obligations, and validation belong in
[CONTRIBUTING.md](../CONTRIBUTING.md).

## Editing discipline

- Format only code touched by the change.
- Do not mix repository-wide formatting or renaming with behavior changes.
- Preserve local vertical layout and naming when editing inherited code.
- Keep reconstruction placeholders such as `func_XXXXXX`, `field_XXX`, and
  `entry_XX` until evidence supports a precise name.

The repository `.clang-format` file is the mechanical reference. Automatic
comment reflow is disabled. Do not run the formatter across unrelated source.

## Language and ownership

New and substantially rewritten C++ targets C++20. Prefer RAII,
standard-library ownership types, explicit initialization, `nullptr`,
`override`, scoped enumerations, and compiler-checked interfaces.

Modernize inherited code incrementally. Before changing types or ownership at
a serialized, network, COM, ABI, deterministic, or hardware-facing boundary,
establish the representation and consumer requirements. Use fixed-width types
where the representation requires them.

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
- Definitions and enumerations live in their own `.hh` headers, which carry
  no static initializations and no nontrivial inline functions.
- Globals and static initializations live in the underscore-prefixed init
  files. [Rationale](RATIONALE.md) records why both conventions exist.
- Preserve external names unless a change deliberately versions or migrates
  the interface.

## Comments and notices

Keep comments sparse: one concise sentence on what the code cannot show — an
invariant, a compatibility constraint, a surprise. Describe the code as it
stands, never the edit that produced it, and never restate what the code
already says. New prose takes `//` or a plain `/* */` block, never the
Westwood `**` decoration; `///` is for genuine XML documentation only, not
the trailing prose much of the inherited tree carries. Comment syntax does
not establish authorship.

Historical file headers and legal notices stay verbatim, a header's
unmaintained `Functions:` table included. Accurate historical comments stay;
correct an inaccurate one narrowly. A function banner that needs substantial
rewriting becomes `///` XML documentation; an ordinary Westwood comment
becomes `//` prose.

Preserve SPDX identifiers, copyright and modification notices, Westwood source
headers, and GPL Section 7 notices. If a notice's history is uncertain, retain
it and investigate before making a legal or attribution change.
