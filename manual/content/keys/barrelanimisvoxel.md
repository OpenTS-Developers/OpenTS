---
key: BarrelAnimIsVoxel
summary: Draws a building's gun barrel from a voxel model over its turret animation.
see_also: ["VoxelBarrelFile", "VoxelBarrelScale", "VoxelBarrelOffsetToBarrelEnd", "TurretAnimIsVoxel", "TurretAnim", "PrimaryFireFLH"]
when_omitted:
  kind: value
  value: "no"
---

Set, the turret stays an animation and the barrel becomes a voxel model drawn over it. The model, [`VoxelBarrelFile`](/keys/voxelbarrelfile/) or a name derived from [`TurretAnim`](/keys/turretanim/), is placed each frame by the four barrel pivot offsets and [`VoxelBarrelScale`](/keys/voxelbarrelscale/), turned to the building's facing and elevated to the barrel's pitch.

```ini title="rules.ini"
[MYARTILLERY] ; a BuildingType registered in [BuildingTypes]
Turret=yes
TurretAnim=MYART_A ; an AnimType registered in [Animations]
BarrelAnimIsVoxel=yes
VoxelBarrelFile=MYARTBAR
VoxelBarrelOffsetToBuildingPivotPoint=4,2,3
VoxelBarrelOffsetToRotatePivotPoint=2,0,0
VoxelBarrelOffsetToPitchPivotPoint=15,0,-8
VoxelBarrelOffsetToBarrelEnd=350,75,0
```

The building draws that turret animation itself rather than leaving it to the layer, so the two keep their order as the gun swings. The barrel is drawn over the animation while the gun points anywhere from north-east round through south to south-west, and under it across the rest of the circle. The barrel is drawn at a fixed depth bias, whatever [`TurretAnimZAdjust`](/keys/turretanimzadjust/) says.

Both the mounting and the muzzle move to the end of the barrel, which [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) covers, in place of the firing offsets [`PrimaryFireFLH`](/keys/primaryfireflh/) supplies.

:::caution[Setting both voxel flags parts the barrel from the shot]
A building that is also [`TurretAnimIsVoxel=yes`](/keys/turretanimisvoxel/) is drawn down the voxel turret path, which places its barrel from [`TurretOffset`](/keys/turretoffset/) and `PrimaryFireFLH` and reads none of the pivot offsets. Its shells still appear at the barrel end those offsets name, so the drawn barrel and the point the shot leaves from no longer agree.
:::
