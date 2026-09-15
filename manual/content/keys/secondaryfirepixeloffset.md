---
key: SecondaryFirePixelOffset
summary: Parsed screen offset that the engine never uses.
no_effect: true
see_also: ["PrimaryFirePixelOffset", "SecondaryFireFLH", "Secondary"]
when_omitted:
  kind: value
  value: 65535,65535
---

The name promises a fixed firing point for the second weapon to match the one the first weapon gets. No path reads it. A building takes its mounting, its muzzle and the point it measures its aim from either from [`PrimaryFirePixelOffset`](/keys/primaryfirepixeloffset/), whichever weapon slot is firing. The per-slot firing offsets apply only when that pair is left at `65535,65535`, and [`SecondaryFireFLH`](/keys/secondaryfireflh/) covers where the second weapon fires from in that case.
