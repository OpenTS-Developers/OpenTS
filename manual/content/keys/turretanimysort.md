---
key: TurretAnimYSort
summary: The bias added to where a building's turret animation falls in the draw order.
see_also: ["TurretAnimY", "TurretAnimZAdjust", "TurretAnim", "TurretAnimIsVoxel"]
when_omitted:
  kind: value
  value: "0"
---

The value is in leptons and is added to the turret animation's sorting position, which decides its place among the objects it shares a layer with. A higher value draws the animation later, over the objects it would otherwise fall behind, and a lower one draws it earlier. Nothing on screen moves: [`TurretAnimX`](/keys/turretanimx/) and [`TurretAnimY`](/keys/turretanimy/) do that. A turret lifted up the screen by `TurretAnimY` is exactly the case this compensates for.

Only a turret drawn as an animation is placed this way. A [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) building has no animation to sort, and a [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building draws its turret animation itself, in step with the barrel, instead of leaving it to the layer.

:::caution[The bias is kept in a single signed byte]
Anything outside `-128` to `127` wraps, so `200` stores as `-56` and biases the animation the other way.
:::
