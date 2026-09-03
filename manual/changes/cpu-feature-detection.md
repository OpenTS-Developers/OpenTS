---
title: Remove MMX and CMOV detection
category: internal
release: 0.2.0
breaking: true
migration:
- Remove any code that calls `Detect_MMX_Availability`, `Detect_CMOV_Availability`, or `Processor`; all have been removed.
- Remove any use of `UseMMX`, `UseCMOV`, and `HasCMOV`; the flags have been removed along with the assembly that read them.
- Drop the MMX flag argument from `Get_CPU_Type` calls; the parameter is gone.
targets: []
credit: [tinix0]
---

OpenTS no longer asks the processor whether it supports MMX or CMOV. This formalizes the
minimum hardware OpenTS already requires — SSE2, so a Pentium 4 or Athlon 64 onward — which
always carries both.

A machine below that minimum was never covered by a build claim. The routines the flags
selected between are C++ now and take one path on every processor, so neither the flags nor the
detection that set them remain. The processor family and vendor CPUID reports are still read
and returned through `Get_CPU_Type` and the `CPUType` and `VendorID` globals.
