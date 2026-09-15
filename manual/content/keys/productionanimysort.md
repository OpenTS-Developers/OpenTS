---
key: ProductionAnimYSort
summary: The sorting bias applied to the production animation, in leptons.
see_also: ["ProductionAnim", "ProductionAnimZAdjust"]
when_omitted:
  kind: value
  value: "0"
---

The bias that moves the production animation within the drawing order of the objects sharing its layer. [Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers how that differs from the depth bias, what the slot does to the AnimType's own sort bias, and the signed byte, -128 through 127, that the figure has to fit in.
