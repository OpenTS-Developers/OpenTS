---
key: VoxelBarrelOffsetToRotatePivotPoint
summary: Where a building's voxel barrel sits once it has turned to face its target.
see_also: ["BarrelAnimIsVoxel", "VoxelBarrelOffsetToBuildingPivotPoint", "VoxelBarrelOffsetToPitchPivotPoint", "VoxelBarrelOffsetToBarrelEnd", "PrimaryFireFLH"]
when_omitted:
  kind: value
  value: "0,0,0"
---

Three whole numbers, `X,Y,Z`. A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building places its barrel with five moves in order. First the assembly moves out from the building by [`VoxelBarrelOffsetToBuildingPivotPoint`](/keys/voxelbarreloffsettobuildingpivotpoint/); a turn to the building's facing comes next, then out by this offset. The barrel then elevates to its pitch and moves out by [`VoxelBarrelOffsetToPitchPivotPoint`](/keys/voxelbarreloffsettopitchpivotpoint/), with the model itself hanging off the end of that.

This one is applied once the assembly has been turned but before it is elevated, so it swings with the gun and holds still as the barrel rises. `X` runs forward along the direction the turret faces, `Y` out to the turret's left and `Z` upward. That is the frame [`PrimaryFireFLH`](/keys/primaryfireflh/) is measured in, and the same place in the sequence that offset occupies for an object with no voxel barrel.

```ini title="rules.ini"
[MYARTILLERY] ; a BuildingType registered in [BuildingTypes]
BarrelAnimIsVoxel=yes
VoxelBarrelOffsetToBuildingPivotPoint=4,2,3
VoxelBarrelOffsetToRotatePivotPoint=2,0,0 ; two units forward of the mount
VoxelBarrelOffsetToPitchPivotPoint=15,0,-8
VoxelBarrelOffsetToBarrelEnd=350,75,0
```

The three pivot offsets are read at two scales. Drawing places the barrel with them as raw model units, while the firing point consumes the same matrix and reads the result as leptons, 256 to a cell. The same value therefore moves the drawn barrel farther than it moves the shot. [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) is the exception to the split: the firing point is the only place it is read, so it is leptons alone.

:::caution[An offset the files never set reads as 0,0,0]
A BuildingType that never assigns this key gets `0,0,0`, and a value that is not three whole numbers is ignored whole, leaving the previous value standing. A `BarrelAnimIsVoxel=yes` building that leaves this offset at that default turns the barrel about the mount point without moving it forward.
:::
