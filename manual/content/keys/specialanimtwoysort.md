---
key: SpecialAnimTwoYSort
summary: The sorting bias applied to the second special slot's animation, in leptons.
see_also: ["SpecialAnimTwo", "SpecialAnimTwoZAdjust", "SpecialAnim"]
when_omitted:
  kind: value
  value: "0"
---

The bias that moves the second special slot's animation within the drawing order of the objects sharing its layer. [Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers how that differs from the depth bias, what the slot does to the AnimType's own sort bias, and the signed byte, -128 through 127, that the figure has to fit in. [`SpecialAnim`](/systems/building-animations/#where-each-setting-is-read-from) covers why it is not written in the same section as the slot's animation names.
