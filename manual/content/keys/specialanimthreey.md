---
key: SpecialAnimThreeY
summary: How far below the structure's drawing point the third special slot's animation sits, in screen pixels.
see_also: ["SpecialAnimThree", "SpecialAnimThreeX", "SpecialAnim"]
when_omitted:
  kind: value
  value: "0"
---

The vertical half of the third special slot's offset. It moves the [`SpecialAnimThree`](/keys/specialanimthree/) animation down the screen from the point the structure is drawn at, and a negative figure lifts it above that point instead. The offset pins the animation to a point on the artwork rather than to a cell, so it holds wherever the structure stands.

[Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers how the offset differs from the two draw-order biases. [Where each setting is read from](/systems/building-animations/#where-each-setting-is-read-from) covers when the two figures are read and why they are not written in the same section as the slot's animation names.
