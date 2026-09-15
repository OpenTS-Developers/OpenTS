---
key: PrimaryFireFLH
summary: The offset from an object's center that its first weapon fires from.
see_also: ["Primary", "Elite", "PBarrelLength", "PBarrelThickness", "TurretOffset", "SecondaryFireFLH"]
when_omitted:
  kind: value
  value: 0,0,0
---

The value is three whole numbers, `X,Y,Z`, each in leptons (256 to a cell). The offset is measured in the turret's frame and turned with it, so it stays fixed relative to the gun rather than to the map. `X` runs forward along the direction the turret faces, `Y` runs out to the turret's left, and `Z` runs upward. A vehicle or infantryman with no turret uses the direction its body faces. A turretless building aims the offset at whatever it is currently firing on, and an aircraft always uses its turret facing whether or not it has a turret.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
PrimaryFireFLH=100,0,60 ; a bit under half a cell forward, 60 leptons up
```

The offset locates two different points. The **mounting** (where the projectile is created and where the firing solution is measured from) is this offset with [`TurretOffset`](/keys/turretoffset/) added to `X`. A building measures it from its own center. A vehicle or aircraft measures it through the same transform its artwork is drawn with, so ground slope and body tilt move the mounting even though they do not move the muzzle. The **muzzle** (where the fire animation, laser beam, sonic wave, and attached particle systems appear) starts from that same point. It adds [`PBarrelThickness`](/keys/pbarrelthickness/) to `Z`, pitches with the barrel, and then runs [`PBarrelLength`](/keys/pbarrellength/) out along the pitched barrel. Only the muzzle rides up as the barrel elevates.

:::caution[An infantry type has no separate mounting]
An InfantryType takes its mounting from the same point as its muzzle, so [`PBarrelThickness`](/keys/pbarrelthickness/) and [`PBarrelLength`](/keys/pbarrellength/) move its projectile as well as its effects.
:::

`Y` is negated on every second shot of a [`Burst`](/keys/burst/) weapon, so a non-zero lateral offset gives a burst weapon a pair of mountings either side of the centerline rather than one.

:::caution[The elite weapon reads this same setting]
The elite slot has no firing offset of its own: it is filled from `PrimaryFireFLH`, `PBarrelLength`, and `PBarrelThickness`, so an [`Elite`](/keys/elite/) weapon always fires from the same point on the artwork as the weapon it replaces.
:::

:::caution[Two building settings replace this offset outright]
A [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building takes both points from its [`VoxelBarrelOffsetToBarrelEnd`](/keys/voxelbarreloffsettobarrelend/) instead, and a building with a [`PrimaryFirePixelOffset`](/keys/primaryfirepixeloffset/) other than `65535,65535` pins both points to that screen offset. Either setting makes this one inert for that building, whichever weapon slot is firing.
:::
