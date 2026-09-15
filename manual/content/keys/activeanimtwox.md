---
key: ActiveAnimTwoX
summary: How far right of the structure's drawing point the second active slot's animation sits, in screen pixels.
see_also: ["ActiveAnimTwo", "ActiveAnimTwoY"]
when_omitted:
  kind: value
  value: "0"
---

The horizontal half of the second slot's offset. It moves the [`ActiveAnimTwo`](/keys/activeanimtwo/) animation to the right of the point the structure is drawn at, and a negative figure moves it left of that point instead. The offset pins the animation to a point on the artwork rather than to a cell, so it holds wherever the structure stands.

[Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers how the offset differs from the two draw-order biases, and [Where each setting is read from](/systems/building-animations/#where-each-setting-is-read-from) covers when the two figures are read.
