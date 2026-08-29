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

Runtime detection of MMX and CMOV support has been removed. `Detect_MMX_Availability`, called
through `Get_CPU_Type`, now unconditionally sets `UseMMX` and returns true instead of reading
the CPUID feature bit, and `UseCMOV` and `HasCMOV` are fixed to true at initialization rather
than written by the removed `Detect_CMOV_Availability`. The processor family CPUID reports is
still read and returned through `Get_CPU_Type`'s `cpu_type` parameter and the `CPUType` global.

`UseMMX` and `UseCMOV` still gate the MMX and CMOV code paths in the palette-fade routines in
`winasm.asm`; those paths are now always taken rather than only on hardware CPUID actually
reports as capable.

`Detect_CMOV_Availability` and `Processor` have been removed outright; neither had a remaining
caller once detection stopped needing them.

This formalizes the minimum hardware OpenTS already requires (SSE2, so a Pentium 4 or Athlon
64 onward), which always carries MMX and CMOV. A machine below that minimum was never covered
by a build claim; running the engine there previously fell back to a slower code path, and now
instead executes an MMX or CMOV instruction the processor does not have.
