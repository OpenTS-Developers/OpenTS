---
key: PBarrelThickness
summary: Raises the point the first weapon's barrel pivots about.
see_also: ["PrimaryFireFLH", "PBarrelLength", "Elite", "SBarrelThickness"]
when_omitted:
  kind: value
  value: "0"
---

The value is in leptons, 256 to a cell, and is added to the `Z` component of [`PrimaryFireFLH`](/keys/primaryfireflh/) before the barrel is pitched. Where [`PBarrelLength`](/keys/pbarrellength/) measures along the barrel, this measures across it: raising the pivot lifts the muzzle onto the middle of the barrel artwork instead of leaving it at the barrel's base.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
PrimaryFireFLH=100,0,60
PBarrelLength=80
PBarrelThickness=12 ; lifts the muzzle onto the barrel's centerline
```

Only the muzzle position moves: the fire animation, laser beam, sonic wave, and attached particle systems. An AircraftType, BuildingType or UnitType creates its projectile at the mounting point, which the barrel does not move; an InfantryType creates it at the muzzle instead.

The elite weapon slot is filled from this same setting, so an [`Elite`](/keys/elite/) weapon fires from the same point as the weapon it replaces.
