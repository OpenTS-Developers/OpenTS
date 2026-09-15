---
key: ProductionAnimX
summary: How far right of the structure's drawing point the production animation sits, in screen pixels.
see_also: ["ProductionAnim", "ProductionAnimY"]
when_omitted:
  kind: value
  value: "0"
---

The horizontal half of the production slot's offset. It moves the [`ProductionAnim`](/keys/productionanim/) animation right across the screen from the point the structure is drawn at, and a negative figure moves it left of that point instead. The offset pins the animation to a point on the artwork rather than to a cell, so it holds wherever the structure stands.

[Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers how the offset differs from the two draw-order biases, and [Where the settings are read](/keys/productionanim/#where-the-settings-are-read) covers which art entry the two figures come from.
