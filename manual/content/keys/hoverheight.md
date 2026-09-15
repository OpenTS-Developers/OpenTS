---
key: HoverHeight
summary: Height in leptons that a hovering unit rides above the ground.
see_also: [HoverBob, HoverDampen, Gravity, "system:emp-pulse"]
when_omitted:
  kind: value
  value: "120"
---

The figure is a plain lepton distance rather than a count of cells. At 256 leptons to a cell and roughly 104 leptons to a terrain level, the stock `120` rides a little over one level off the ground, and `208` rides two. Each frame, while a powered hover unit's clearance, its height above the ground, is under this figure, the cushion adds `(2 × HoverHeight − clearance) ÷ HoverHeight` times [`Gravity`](/keys/gravity/) to its vertical motion. `Gravity` is taken off again every frame. The lift and the fall therefore balance just below this figure, and the unit settles there rather than being held at it. Under a quarter of the height a further `Gravity ÷ 3` is added, truncated to whole leptons, so a `Gravity` below 3 contributes nothing here. That kick is given whether the unit has power or not.

While the next step of the unit's path leads to a higher cell, the cushion measures the clearance as this much lower. It therefore begins lifting a cell before the unit reaches the rise.

The levitation locomotor, the one the stock Tiberium floater uses, floats on the same figure.

:::caution[An unpowered hover unit sinks]
The proportional lift is added only while the locomotor has power, and the quarter-height kick alone does not cover the `Gravity` taken off each frame. An [EMP pulse](/systems/emp-pulse/), or an ion storm for an ion-sensitive type, therefore abandons the unit's move order and gives it a shove. The unit then sags to the ground and tilts onto the ramp beneath it until power returns.
:::
