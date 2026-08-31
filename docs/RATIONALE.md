# Rationale

This document explains why the reconstructed source looks the way it does: the
tools used, how the original structure was recovered, and choices that may
otherwise look accidental. It grows as the same questions recur.
[History](HISTORY.md) records the source's origin, and
[Project direction](DIRECTION.md) covers the engine's future.

## Reconstruction tools

The reconstruction used:

- The original compiler, or the closest known match. For MSVC 6 and
  later, the executable's Rich header identifies the build tools.
- An interactive disassembler and decompiler that turns assembly into
  pseudo-C and keeps a persistent database of recovered names and types.
- A tool that builds object files from extracted assembly or directly from the
  binary.
- [objdiff](https://github.com/encounter/objdiff), which compares those
  objects with objects compiled from the reconstructed source.

## Recovering module file names

Most file names survive in the Red Alert and Renegade source releases.
Tiberian Sun through Yuri's Revenge also print many names in debug and error
output. The rest are informed guesses, but they can be made with some
precision. The MSVC IDE sorts project files alphabetically, and the linker
preserves that module order. A module's known neighbors and purpose narrow the
possible names, and inserting a candidate into that order quickly shows
whether it fits.

## Underscore-prefixed file names

The Visual Studio solution groups `code/` files such as `_map.cpp`,
`_rules.cpp`, and `_mixfile.cpp` as init files. They contain globals and
static initializations.

Many globals and static initializations occur at the start of the binary. Red
Alert keeps most of them in `globals.h` and `globals.cpp`; Tiberian Sun split
them across more files. Since the IDE and linker use alphabetical module
order, their position implies names that sort before ordinary source files. A
leading underscore is the likely prefix. Released Red Alert and Renegade
sources confirm this convention, from `_WSPROTO.CPP` beside `WSPROTO.CPP` in
Red Alert to the many underscore-prefixed files in Renegade's `wwlib`.

## Definition headers: the `.hh` files

Red Alert puts nearly all definitions in one `defines.h`. The reconstruction
instead gives each set of definitions its own `.hh` file. These files are
intended for definitions. New `.hh` files should avoid static initialization
and nontrivial inline code.

This keeps the cost of including a type small. A normal class header makes the
compiler process everything else that header contains, even when a file needs
only one enumeration or type. A definition-only header avoids carrying that
work across every file that uses the definition.

Comments in `rules.ini` refer to definition and enumeration files as `.hh`,
which is why the reconstruction uses that extension instead of `.hpp`.

## Recovering dialog, control, and string names

Language-resource names come from Red Alert 2. Its different dialog system
exposes likely dialog and control IDs. Some versions of both games also print
these IDs in debug output.

Red Alert 2 indexes strings by label and stores those labels in its string
table. Cross-referencing that table with Tiberian Sun's string table recovers
the original string defines, except for Firestorm additions absent from Red
Alert 2.

## Assigning code to modules

The original Tiberian Sun code placed static initializations in headers, so
many modules contain duplicate initializations of the same globals. Each new
set marks the start of another module's functions and data, allowing code to be
assigned to modules with high confidence.

## Why the binary does not match exactly

The reconstruction was not intended to reproduce the exact 2.03 executable.
Doing so would require its missing DBG or PDB symbols; without them, some of
the necessary information is unrecoverable. The goal was instead to match
every function, its referenced data, and loose data found during analysis.
Some unreferenced data may remain undiscovered.

[History](HISTORY.md#function-level-matching) records the resulting match
statistics. Most data sections also differ, and some functions may never
match. A future Red Alert 2 decompilation may still provide evidence for more
of them.

The remaining mismatches have several causes:

- MSVC of that era is inconsistent, and nearby code can affect its output.
- One missing or extra inline can prevent a function from matching.
- `static` affects code generation even when a `static const` value is
  optimized away. In `Dropship_Screen`, adding the correct values raised the
  match from 65% to 98%. Such constants can often be found as unreferenced
  data in the binary.
- An object file's BSS section is sorted by a hash of each symbol's name, so
  reproducing its order usually requires the original symbol names. When the
  hash is known, equivalent names can be guessed or brute-forced.
- The compiler may swap vtables or change RTTI entry flags unpredictably.
- Include order changes code and data order, especially for inlines, vtables,
  and other compiler-generated data.

Two divergences were deliberate. The duplicate static initializations
described under [Assigning code to modules](#assigning-code-to-modules) were
removed because they made the reconstruction harder to work with. Includes
also follow a fixed sort order, even where that changes a module's original
data order. Exact executable matching was out of scope, so both tradeoffs were
accepted.
