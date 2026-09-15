---
key: TurretAnimY
summary: The vertical screen offset from a building's artwork to its turret.
see_also: ["TurretAnimX", "TurretAnim", "TurretAnimIsVoxel", "TurretAnimYSort", "TurretAnimZAdjust"]
when_omitted:
  kind: value
  value: "0"
---

The offset is in screen pixels, measured from the point the building's own artwork is drawn at; a positive value moves the turret down the screen and a negative one up it. It places the turret animation, and on a building drawn with a voxel turret or a voxel barrel it places that model.

```ini title="rules.ini"
[MYTOWER] ; a BuildingType registered in [BuildingTypes]
TurretAnim=MYTOWER_A ; an AnimType registered in [Animations]
TurretAnimX=-2
TurretAnimY=10 ; the turret sits two pixels left of the draw point and ten below it
```

The pair is more than a drawing offset. It is projected back onto the ground, so the direction the building aims and fires along is measured from the displaced point rather than from its center. On a [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) or [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building the shot is created there too.

Because the offset lands on the ground rather than in the air, a turret animation lifted up the screen by a negative value also sorts as though it stood that much further back. It can fall behind objects it used to be drawn over. [`TurretAnimYSort`](/keys/turretanimysort/) covers biasing that sorting position back without moving the artwork again.
