---
key: TurretAnimZAdjust
summary: The depth bias a building's turret art is drawn with.
see_also: ["TurretAnimYSort", "TurretAnim", "TurretAnimIsVoxel", "BarrelAnimIsVoxel"]
when_omitted:
  kind: value
  value: "0"
---

The value is in pixels of depth. A lower value draws the art in front of what it overlaps and a higher one behind it. Turret art is normally given a negative bias, because it has to reach over the structure it is mounted on. The key is the whole bias: the engine adds nothing to the value set here.

Which art it biases depends on how the turret is drawn. A turret animation takes the bias itself. A [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) building has no animation and writes its voxel turret and barrel at this depth instead. A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building writes its voxel barrel at a fixed bias of 21 pixels in front and reads this key for its turret animation alone.

:::caution[The bias is kept in a single signed byte]
Anything outside `-128` to `127` wraps, so `-200` stores as `56` and pushes the art behind the building rather than in front of it.
:::
