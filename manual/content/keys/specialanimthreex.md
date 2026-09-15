---
key: SpecialAnimThreeX
summary: How far right of the structure's drawing point the third special slot's animation sits, in screen pixels.
see_also: ["SpecialAnimThree", "SpecialAnimThreeY", "SpecialAnim"]
when_omitted:
  kind: value
  value: "0"
---

The horizontal half of the third special slot's offset. It moves the [`SpecialAnimThree`](/keys/specialanimthree/) animation right across the screen from the point the structure is drawn at, and a negative figure moves it left of that point instead. The offset pins the animation to a point on the artwork rather than to a cell, so it holds wherever the structure stands.

[Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers how the offset differs from the two draw-order biases. [Where each setting is read from](/systems/building-animations/#where-each-setting-is-read-from) covers when the two figures are read and why they are not written in the same section as the slot's animation names.
