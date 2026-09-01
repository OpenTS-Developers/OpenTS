---
title: Assume MMX and CMOV on the supported minimum hardware
category: internal
release: 0.2.0
breaking: true
migration:
- Remove any code that calls `Detect_CMOV_Availability` or `Processor`; both have been removed.
- Treat `UseMMX`, `UseCMOV`, and `HasCMOV` as fixed-true constants rather than runtime-detected flags.
targets: []
credit: [tinix0]
---

OpenTS no longer asks the processor at startup whether it supports MMX and CMOV, and assumes
both. This formalizes the minimum hardware OpenTS already requires — SSE2, so a Pentium 4 or
Athlon 64 onward — which always carries MMX and CMOV.

A machine below that minimum was never covered by a build claim. It previously took a slower
path through the palette-fade routines and now executes an MMX or CMOV instruction the
processor does not have. The processor family CPUID reports is still read and returned through
`Get_CPU_Type` and the `CPUType` global.
