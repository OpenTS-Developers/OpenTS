---
key: MaxZVel
summary: The top of the range a voxel animation's upward launch speed is drawn from, in leptons per frame.
see_also: ["MinZVel", "MaxXYVel", "IsMeteor"]
when_omitted:
  kind: value
  value: "5"
---

Leptons per frame: 256 leptons to a cell, 15 frames to the second. The vertical launch speed is [`MinZVel`](/keys/minzvel/#scope-voxelanimtype) plus a whole number of leptons drawn from the truncated span between the two settings. The speeds available are the minimum and whole steps above it, and this figure is itself reached only where it stands a whole number of leptons above the minimum. A minimum of `3.5` against a maximum of `5` truncates to a span of two and offers exactly `3.5` and `4.5`.

A meteor ignores the setting entirely: its vertical speed is the minimum, exactly.

:::danger[A maximum just below the minimum divides by zero]
The divisor the pick uses is the truncated span plus one. A maximum below the minimum by less than two makes that divisor zero, the division faults, and the game stops the moment a piece of the type is created. A meteor never makes this pick and is unaffected. A maximum lower still is accepted: the pick runs against a negative divisor and still returns a non-negative offset, so the speeds run upward from the minimum, over a narrower band than the same gap upward would give.
:::
