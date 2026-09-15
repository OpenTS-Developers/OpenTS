---
key: BestLowPowerBuildRateCoefficient
summary: Parsed best-case low-power build coefficient that the engine never uses.
no_effect: true
see_also: ["system:power"]
when_omitted:
  kind: value
  value: ".75"
---

The gentlest production multiplier a shortfall can impose is fixed in the engine at `0.75`, the same figure this default names. [The production ladder](/systems/power/#production) lists the steps that are actually applied.
