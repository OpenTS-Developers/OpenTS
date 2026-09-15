---
key: VoxelBarrelOffsetToBarrelEnd
summary: Where a building with a voxel barrel creates its shots.
see_also: ["BarrelAnimIsVoxel", "VoxelBarrelScale", "VoxelBarrelOffsetToPitchPivotPoint", "PrimaryFireFLH", "Burst"]
when_omitted:
  kind: value
  value: "0,0,0"
---

Three whole numbers, `X,Y,Z`, in leptons: 256 to a cell. The offset runs through the same placement the drawn barrel does, so it is measured on the pitched gun: `X` out along the barrel, `Y` to its left and `Z` at right angles to both. [`VoxelBarrelScale`](/keys/voxelbarrelscale/) multiplies all three before the pivot offsets take it out to the structure. It is the only one of the four offsets that nothing draws.

On a [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building it replaces both points that [`PrimaryFireFLH`](/keys/primaryfireflh/) would otherwise fix: the mounting and the muzzle. The mounting is where the projectile is created and the firing solution is measured from; the muzzle is where the fire animation and beam appear. The two are the same point here, so the shot and its flash always leave the barrel together, and both rise as the gun elevates.

```ini title="rules.ini"
[MYARTILLERY] ; a BuildingType registered in [BuildingTypes]
BarrelAnimIsVoxel=yes
VoxelBarrelScale=.75
VoxelBarrelOffsetToBarrelEnd=350,75,0 ; drawn at .75, the muzzle sits 262 leptons out
```

A [`Burst`](/keys/burst/) weapon alternates: the first shot of a burst uses `Y` as written and the second mirrors it, giving a pair of muzzles either side of the barrel. A third shot and any after it are centered on the barrel instead.

:::caution[An offset the files never set reads as 0,0,0]
A BuildingType that never assigns this key gets `0,0,0`, and a value that is not three whole numbers is ignored whole, leaving the previous value standing. A `BarrelAnimIsVoxel=yes` building that leaves this offset at that default creates its shots at the barrel's pitch pivot instead of out along the barrel.
:::
