---
key: VoxelBarrelOffsetToBuildingPivotPoint
summary: Where a building's voxel barrel assembly is mounted on the structure.
see_also: ["BarrelAnimIsVoxel", "VoxelBarrelOffsetToRotatePivotPoint", "VoxelBarrelOffsetToPitchPivotPoint", "VoxelBarrelOffsetToBarrelEnd", "VoxelBarrelScale"]
when_omitted:
  kind: value
  value: "0,0,0"
---

Three whole numbers, `X,Y,Z`. A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building places its barrel with five moves in order. First the assembly moves out from the building by this offset; a turn to the building's facing comes next, then out by [`VoxelBarrelOffsetToRotatePivotPoint`](/keys/voxelbarreloffsettorotatepivotpoint/). The barrel then elevates to its pitch and moves out by [`VoxelBarrelOffsetToPitchPivotPoint`](/keys/voxelbarreloffsettopitchpivotpoint/), with the model itself hanging off the end of that.

This one is applied before the turn, so it holds still while the gun swings and its axes are the map's rather than the gun's. `X` runs toward the lower right of the screen, `Y` toward the upper right, and `Z` upward. It is the offset that seats the whole assembly on the structure.

```ini title="rules.ini"
[MYARTILLERY] ; a BuildingType registered in [BuildingTypes]
BarrelAnimIsVoxel=yes
VoxelBarrelOffsetToBuildingPivotPoint=4,2,3 ; the mount, drawn an eighth of a cell out
VoxelBarrelOffsetToRotatePivotPoint=2,0,0
VoxelBarrelOffsetToPitchPivotPoint=15,0,-8
VoxelBarrelOffsetToBarrelEnd=350,75,0
```

The three pivot offsets are read at two scales. Drawing places the barrel with them as raw model units, while the firing point consumes the same matrix and reads the result as leptons, 256 to a cell. The same value therefore moves the drawn barrel farther than it moves the shot. [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) is the exception to the split: the firing point is the only place it is read, so it is leptons alone.

:::caution[An offset the files never set reads as 0,0,0]
A BuildingType that never assigns this key gets `0,0,0`, and a value that is not three whole numbers is ignored whole, leaving the previous value standing. A `BarrelAnimIsVoxel=yes` building that leaves this offset at that default mounts the whole barrel assembly on the structure's own origin.
:::
