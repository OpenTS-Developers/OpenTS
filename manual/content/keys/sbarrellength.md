---
key: SBarrelLength
summary: How far out along the second weapon's barrel its muzzle sits.
see_also: ["SecondaryFireFLH", "SBarrelThickness", "PBarrelLength"]
when_omitted:
  kind: value
  value: "0"
---

The distance is in leptons, 256 to a cell. It is applied after the barrel has been pitched, so the muzzle swings with the gun while the mounting point stays put. It affects only the second weapon slot, and only the muzzle position: the fire animation, laser beam, sonic wave, and attached particle systems. The projectile is created at the mounting point that [`SecondaryFireFLH`](/keys/secondaryfireflh/) and [`TurretOffset`](/keys/turretoffset/) fix. An AircraftType, BuildingType or UnitType creates its projectile at the mounting point, which the barrel does not move; an InfantryType creates it at the muzzle instead.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
SecondaryFireFLH=100,0,20
SBarrelLength=40 ; the second weapon's muzzle sits 40 leptons out along its barrel
```
