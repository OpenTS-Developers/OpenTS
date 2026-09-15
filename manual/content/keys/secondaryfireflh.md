---
key: SecondaryFireFLH
summary: The offset from an object's center that its second weapon fires from.
see_also: ["Secondary", "SBarrelLength", "SBarrelThickness", "TurretOffset", "PrimaryFireFLH"]
when_omitted:
  kind: value
  value: 0,0,0
---

The three components have the same meaning and the same units as [`PrimaryFireFLH`](/keys/primaryfireflh/), measured in the turret's frame: `X` forward, `Y` to the turret's left, `Z` upward, in leptons.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
PrimaryFireFLH=100,0,60
SecondaryFireFLH=100,0,20 ; the second weapon fires from lower down the hull
```

This offset applies only while the second weapon slot is the one firing. The elite substitution never reaches the second slot, so nothing else reads this setting. [`SBarrelLength`](/keys/sbarrellength/) and [`SBarrelThickness`](/keys/sbarrelthickness/) extend it into a muzzle position exactly as the primary pair do.

:::caution[Two building settings replace this offset outright]
A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building takes its firing points from its [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) instead, and a building with a [`PrimaryFirePixelOffset`](/keys/primaryfirepixeloffset/) other than `65535,65535` pins them to that screen offset. Either setting makes this one inert for that building, including for its second weapon.
:::
