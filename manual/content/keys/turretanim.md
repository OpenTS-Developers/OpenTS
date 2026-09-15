---
key: TurretAnim
summary: The animation a building's turret is drawn from, or the voxel model that replaces it.
see_also: ["TurretAnimDamaged", "TurretAnimIsVoxel", "BarrelAnimIsVoxel", "VoxelBarrelFile", "TurretAnimX", "Turret"]
when_omitted:
  kind: value
  value: ""
---

Unless [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) is set, the value is an AnimType ID, and that animation is attached to the building's turret slot once the structure is finished, placed at the offset [`TurretAnimX`](/keys/turretanimx/) and [`TurretAnimY`](/keys/turretanimy/) give. Its frame is chosen again every game frame: a turret-equipped building shows the frame matching the direction its turret points, one of 32, while a [`ChargeAnim=yes`](/keys/chargeanim/) building shows the frame its charge sequence has reached instead.

With `TurretAnimIsVoxel=yes` nothing is attached to the turret slot and the value is a voxel base name instead. With [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) alone the value stays an AnimType ID and the animation is still attached: the building simply draws it itself rather than leaving it to the layer. The voxel name is derived under either flag, `<name>.VXL` with the `<name>.HVA` beside it. It is loaded as the turret model only when `TUR` appears in it at the fifth character or later. The scan never looks at the first four characters, so a name that begins with `TUR` counts as having none. The barrel model's name is then that same name with the `TUR` and everything after it replaced by `BARL`, and [`VoxelBarrelFile`](/keys/voxelbarrelfile/) is not read. Without that `TUR` no turret model is loaded at all, and a `TurretAnimIsVoxel=yes` building draws the named model as its barrel, as one piece that both turns and elevates.

```ini title="rules.ini"
[MYTOWER] ; a BuildingType registered in [BuildingTypes]
Turret=yes
TurretAnimIsVoxel=yes
TurretAnim=MYTWRTUR ; drawn from MYTWRTUR.VXL, its barrel from MYTWRBARL.VXL
```

The building named by [`WallTower`](/keys/walltower/) starts from this setting like any other, but its first upgrade takes over. From then on its turret animation is its own Image ID with `_B`, `_C` or `_D` appended. The letter advances by one each time an upgrade is plugged in and wraps back to `_B` after the third.

:::danger[A charging weapon crashes without this animation]
A building whose primary weapon is [`Charges=yes`](/keys/charges/) reaches into its turret animation the moment it begins to charge, without checking that it has one. If this key is unset, or names an animation that is not registered, the game crashes as that building starts charging at its first target.
:::
