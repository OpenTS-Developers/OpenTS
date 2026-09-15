---
key: MaxAngularVelocity
summary: The top of the range a voxel animation's tumbling rate is drawn from, in degrees per frame.
see_also: ["MinAngularVelocity", "Duration"]
when_omitted:
  kind: value
  value: "-1"
---

Degrees per frame, converted to radians as it is read. A piece is given one tumbling rate for the whole of its life, drawn once when it is created from the range this setting closes and [`MinAngularVelocity`](/keys/minangularvelocity/) opens.

A value of exactly `0` is discarded rather than stored. The conversion is skipped and the rate already held stands, which for a type no earlier file touched is ten degrees per frame. Every other value, negative ones included, is taken as written.

:::caution[The pick is quantized in whole radians]
The range is measured after the conversion to radians, and the pick that spans it moves in whole radians. A range narrower than about 57 degrees therefore always yields the minimum, and this setting changes nothing at all. A wider range adds whole radians, about 57 degrees per frame apiece, as the only alternatives to it.
:::

:::danger[A maximum below the minimum usually crashes the game]
The divisor the pick uses is the truncated whole number of radians in the range, plus one. A maximum below the minimum by less than about 114 degrees makes that divisor zero, the division faults, and the game stops the moment a piece of the type is created. Leaving this setting out of a section that writes a minimum reaches the same fault, because an unwritten maximum is minus one degree per frame.
:::
