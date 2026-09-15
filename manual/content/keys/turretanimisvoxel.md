---
key: TurretAnimIsVoxel
summary: Draws a building's turret from a voxel model rather than from an animation.
see_also: ["TurretAnim", "BarrelAnimIsVoxel", "TurretNotExportedOnGround", "TurretAnimZAdjust", "TurretOffset", "PrimaryFireFLH"]
when_omitted:
  kind: value
  value: "no"
---

Set, nothing is attached to the building's turret slot. The models named by [`TurretAnim`](/keys/turretanim/) are drawn over the structure each frame at the pixel offset [`TurretAnimX`](/keys/turretanimx/) and [`TurretAnimY`](/keys/turretanimy/) give, at the depth [`TurretAnimZAdjust`](/keys/turretanimzadjust/) gives them. The mounting and muzzle move by that same pixel offset, on top of the [`PrimaryFireFLH`](/keys/primaryfireflh/) they are built from. The building also sorts as though it stood 32 leptons further south, which lifts the turret over neighbors that would otherwise be drawn across it.

Aiming stops being confined to the 32 frames a turret animation has. A turret drawn from an animation may fire as soon as its facing is within 11.25 degrees of the aim direction (one facing's worth), and its shot leaves along that rounded facing. A voxel turret must be pointed exactly at the aim direction before it may fire, and its shot leaves along the exact direction to the target. A turret that has come within 2.8125 degrees is snapped the rest of the way so that it can fire at all.

```ini title="rules.ini"
[MYTOWER] ; a BuildingType registered in [BuildingTypes]
Turret=yes
TurretAnim=MYTWRTUR ; the voxel base name, not an AnimType ID
TurretAnimIsVoxel=yes
TurretAnimZAdjust=-40
```

The barrel model that comes with the turret is placed from [`TurretOffset`](/keys/turretoffset/) and `PrimaryFireFLH`, and [`TurretNotExportedOnGround`](/keys/turretnotexportedonground/) covers which point it pitches about. The four barrel pivot offsets, beginning with [`VoxelBarrelOffsetToBuildingPivotPoint`](/keys/voxelbarreloffsettobuildingpivotpoint/), and [`VoxelBarrelScale`](/keys/voxelbarrelscale/) take no part in this: a [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building is the only kind that puts them to work.
