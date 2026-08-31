# History

This document records where the OpenTS source came from and how it was
reconstructed. The controlling license and additional terms are in
[LICENSE.md](../LICENSE.md), and
[ACKNOWLEDGEMENTS.md](../ACKNOWLEDGEMENTS.md) credits the people and projects
named here.

## The original executable

Westwood Studios released *Command & Conquer: Tiberian Sun* in 1999 and the
*Firestorm* expansion in 2000. Compiling a large C++ game discards most of what
makes its source understandable: names, types, comments, file structure, and
intent. For decades, changing Tiberian Sun therefore meant interpreting
disassembly or decompiled pseudocode, inferring the intended mechanics, and
patching the retail executable. This enabled years of community work, but
changes remained expensive and fragile.

OpenTS replaces that process with ordinary source development: read the
implementation, edit it, compile it, and test the result.

The reconstruction's reference target is the latest English (US) executable:

```text
GAME.EXE; v2.03[EN]; Monday 5th June, 2000 (21:26:42)
MD5: C2C58CBBF83AF0458DC44EF64A3C011F
```

## Source foundation

Westwood shared code and libraries across games. Electronic Arts' source
releases for related Command & Conquer titles therefore contain much of
Tiberian Sun's foundation, but not its finished engine. The reconstruction
began with the GPL-licensed Red Alert source from the
[2020 Command & Conquer Remastered Collection](https://github.com/electronicarts/CnC_Remastered_Collection),
then moved to the fuller
[Red Alert source](https://github.com/electronicarts/CnC_Red_Alert) released
with The Ultimate Collection in 2025. It also uses applicable shared code from
the
[Tiberian Dawn](https://github.com/electronicarts/CnC_Tiberian_Dawn),
[Renegade](https://github.com/electronicarts/CnC_Renegade), and
[Generals and Zero Hour](https://github.com/electronicarts/CnC_Generals_Zero_Hour)
repositories.

Missing Tiberian Sun code and behavior were reverse engineered from those
public releases and the shipped game. No other Westwood source material was
used. Applicable Electronic Arts notices remain on derived files, and that
material is subject to the additional GPL Section 7 terms in
[LICENSE.md](../LICENSE.md).

## Function-level matching

The current reconstruction began in 2024. Functions were reconstructed in C++,
compiled with the historical compiler environment, and compared instruction by
instruction with the reference executable. The work used disassemblers,
decompilers, objdiff, custom build and comparison tools, and runtime testing.
For an individual function, exact machine-code matching is stronger evidence
than observed feature behavior. Runtime testing still matters for whole-game
behavior.

All roughly 11,150 known functions are implemented. Around 500 produce
different instruction sequences and have not been proven identical, though no
user-visible divergence is known. The archived baseline has approximately 98%
weighted, penalty-adjusted instruction-level matching. This function-level
score does not measure byte similarity or feature completion, and it is not the
percentage of exact matches. [Rationale](RATIONALE.md) describes the tools,
recovered module layout, and causes of the remaining mismatches.

Codex, Claude, and DeepSeek accelerated the final matching work. They proposed
interpretations and candidate implementations from disassembly, decompiled
pseudocode, nearby reconstructed source, compiler errors, and binary-diff
feedback. Developers reviewed and compiled every candidate, then compared it
against the reference executable.

## From TibSun to OpenTS

Two repositories preserve the reconstruction for deliberately different
purposes.

The [TibSun archive](https://github.com/OpenTS-Developers/TibSun) freezes the
cleaned, code-only matching baseline for historical reference, further
matching work, and comparison with the original game. OpenTS derives from that
source baseline; this describes its provenance, not its Git history, and the
archive is not a Git ancestor of this repository.

OpenTS is the active project. It uses C++20, CMake, and Visual Studio 2022;
this repository builds releases, accepts contributions, and hosts the manual.
As OpenTS diverges from the original executable, binary matching changes role.
It remains the strongest evidence of inherited behavior and establishes the
starting point for a change, but it is not an acceptance gate. Project goals
and compatibility determine whether the result is accepted.

OpenTS published its first release, 0.1.0, on 27 August 2026.
