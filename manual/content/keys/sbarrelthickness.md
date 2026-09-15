---
key: SBarrelThickness
summary: Raises the point the second weapon's barrel pivots about.
see_also: ["SecondaryFireFLH", "SBarrelLength", "PBarrelThickness"]
when_omitted:
  kind: value
  value: "0"
---

The value is in leptons, 256 to a cell. It is added to the `Z` component of [`SecondaryFireFLH`](/keys/secondaryfireflh/) before the barrel is pitched, lifting the muzzle onto the middle of the barrel artwork rather than its base. It affects only the second weapon slot, and only the muzzle position: the fire animation, laser beam, sonic wave, and attached particle systems. An AircraftType, BuildingType or UnitType creates its projectile at the mounting point, which the barrel does not move; an InfantryType creates it at the muzzle instead.

```ini title="art.ini"
[MYTANK] ; the Image ID of a UnitType
SecondaryFireFLH=100,0,20
SBarrelLength=40
SBarrelThickness=8
```
