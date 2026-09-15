---
key: LaserFence
summary: Whether the structure is a laser fence segment that a fence post lays, energizes and removes.
see_also: ["system:laser-fences"]
when_omitted:
  kind: value
  value: "no"
---

A segment is never built or paid for. A [`LaserFencePost=yes`](/keys/laserfencepost/) structure creates one segment per cell along a [run it lays toward another post](/systems/laser-fences/#laying-the-run), switches the whole run between its live and slack shapes, and deletes or destroys it again. Only the first declared BuildingType with this flag is ever selected when a post lays a run; a second one is never created that way.

The flag also changes what the type may stand on and what may stand on it. A segment needs a cell holding no building and no terrain object. Tiberium or veins in that cell, which would otherwise refuse the placement, are allowed under **All of:**

- the cell is not a bridge;
- it was not under one;
- it is not a ramp.

A laser fence post or a gate may then be placed on top of a segment of the same house, which no other structure may do.

:::caution[A segment cannot be shot away]
Damage that is not forced returns no result at all on this type, so ordinary weapon fire and splash leave a segment untouched however low its [`Strength`](/keys/strength/). Only the owning post's teardown, a structure placed on the cell, or a forced kill removes one.
:::
