---
key: CliffRamps
summary: Tile set holding the theater's twenty cliff ramp pieces.
see_also: [CliffSet, WaterCliffs, RampBase, SlopeSetPieces]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and the generator's cliff test then treats the theater's first nineteen tiles as cliff ramps.
---

The twenty pieces from this role count as cliff, and that is the whole of what the setting does. The cliff test that random map generation runs treats a tile in this range exactly as it treats one from [`CliffSet`](/keys/cliffset/). The generator neither counts a cliff ramp as ground when it decides which cells to raise nor lays a shore piece over one. Cliff ramps are rock face, not the walkable ramps of [`RampBase`](/keys/rampbase/). [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

Whether a cliff ramp can be crossed is settled by the land type its artwork has, not by this setting, so leaving the role unresolved changes generation alone.
