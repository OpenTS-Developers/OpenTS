---
key: SpecialAnimThreeZAdjust
summary: The depth bias applied to the third special slot's animation.
see_also: ["SpecialAnimThree", "SpecialAnimThreeYSort", "SpecialAnim"]
when_omitted:
  kind: value
  value: "0"
---

The bias that decides whether the third special slot's animation is drawn over the structure or behind it. [Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers which direction each sign moves it and the signed byte, -128 through 127, that the figure has to fit in. [`SpecialAnim`](/systems/building-animations/#where-each-setting-is-read-from) covers why it is not written in the same section as the slot's animation names.
