---
key: ProjectileRange
summary: How far a fueled projectile may fly before it detonates wherever it happens to be, in cells.
see_also: ["Ranged", "Projectile", "Range"]
when_omitted:
  kind: value
  value: "390.625"
---

Only a projectile marked [`Ranged=yes`](/keys/ranged/) burns fuel. Such a shot starts with this figure and subtracts the distance it covers each time it moves. The moment it reaches nothing it detonates where it stands, which is what stops a missile chasing an evading target across the map. Every other projectile holds the figure and never spends it. The figure always comes from the weapon that fires the shot, so two weapons sharing one projectile still give their shots their own figures.

```ini title="rules.ini"
[MyMissile] ; example WeaponType
Range=12
ProjectileRange=14 ; the missile outlasts the shot it was fired for by two cells
Projectile=MyRocket ; a BulletType, registered by a weapon naming it as its Projectile

[MyRocket] ; example BulletType
Ranged=yes
ROT=20
```

The value is written in cells and a fraction is accepted; it is held as 256 units to the cell. The stored default of 100000 units is a little over 390 cells, far past the width of any map. A fueled projectile with the key unwritten therefore behaves as though it had no fuel limit at all.

A figure of exactly `-1` is read as though the key were absent, leaving whatever an earlier rules file set.

Not every shot draws its fuel from here. A bomblet thrown by an [`AirburstWeapon=`](/keys/airburstweapon/) takes that weapon's [`Range=`](/keys/range/) as its fuel instead. The two trigger actions that launch a cluster missile and a chemical missile take the figure from the weapons named `MultiLauncher` and `ChemLauncher` wherever those are configured.
