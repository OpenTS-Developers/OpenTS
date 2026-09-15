---
key: PBarrelLength
summary: How far out along the first weapon's barrel its muzzle sits.
see_also: ["PrimaryFireFLH", "PBarrelThickness", "Elite", "SBarrelLength"]
when_omitted:
  kind: value
  value: "0"
---

The distance is in leptons, 256 to a cell. It is applied after the barrel has been pitched, so the muzzle swings up and down with the gun while the mounting point stays put.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
PrimaryFireFLH=100,0,60
PBarrelLength=80 ; the muzzle sits 80 leptons further out along the barrel
```

Only the muzzle position moves: the fire animation, laser beam, sonic wave, and attached particle systems. The projectile itself is created at the mounting point that [`PrimaryFireFLH`](/keys/primaryfireflh/) and [`TurretOffset`](/keys/turretoffset/) fix. An AircraftType, BuildingType or UnitType creates its projectile at the mounting point, which the barrel does not move; an InfantryType creates it at the muzzle instead.

The elite weapon slot is filled from this same setting, so an [`Elite`](/keys/elite/) weapon fires from the end of the same barrel as the weapon it replaces.
