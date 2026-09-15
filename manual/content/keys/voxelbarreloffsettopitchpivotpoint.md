---
key: VoxelBarrelOffsetToPitchPivotPoint
summary: Where a building's voxel barrel sits once it has elevated.
see_also: ["BarrelAnimIsVoxel", "VoxelBarrelOffsetToBuildingPivotPoint", "VoxelBarrelOffsetToRotatePivotPoint", "VoxelBarrelOffsetToBarrelEnd", "PBarrelLength"]
when_omitted:
  kind: value
  value: "0,0,0"
---

Three whole numbers, `X,Y,Z`. A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building places its barrel with five moves in order. First the assembly moves out from the building by [`VoxelBarrelOffsetToBuildingPivotPoint`](/keys/voxelbarreloffsettobuildingpivotpoint/); a turn to the building's facing comes next, then out by [`VoxelBarrelOffsetToRotatePivotPoint`](/keys/voxelbarreloffsettorotatepivotpoint/). The barrel then elevates to its pitch and moves out by this offset, with the model itself hanging off the end of that.

This one is applied after the elevation, so it rides up and down with the gun. `X` runs out along the pitched barrel, `Y` to its left and `Z` at right angles to both. That is the place in the sequence [`PBarrelLength`](/keys/pbarrellength/) occupies for an object with no voxel barrel, with two more components to it. The point it lands on is where the barrel model is hung, so it is the offset that decides which part of the barrel stays still while the rest swings.

```ini title="rules.ini"
[MYARTILLERY] ; a BuildingType registered in [BuildingTypes]
BarrelAnimIsVoxel=yes
VoxelBarrelOffsetToBuildingPivotPoint=4,2,3
VoxelBarrelOffsetToRotatePivotPoint=2,0,0
VoxelBarrelOffsetToPitchPivotPoint=15,0,-8 ; forward along the barrel and slightly below it
VoxelBarrelOffsetToBarrelEnd=350,75,0
```

The three pivot offsets are read at two scales. Drawing places the barrel with them as raw model units, while the firing point consumes the same matrix and reads the result as leptons, 256 to a cell. The same value therefore moves the drawn barrel farther than it moves the shot. [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) is the exception to the split: the firing point is the only place it is read, so it is leptons alone.

:::caution[An offset the files never set reads as 0,0,0]
A BuildingType that never assigns this key gets `0,0,0`, and a value that is not three whole numbers is ignored whole, leaving the previous value standing. A `BarrelAnimIsVoxel=yes` building that leaves this offset at that default pitches the barrel about the model's own origin.
:::
