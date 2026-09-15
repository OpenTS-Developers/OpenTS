---
key: Airburst
summary: Holds a homing projectile at cruising height so that it goes off above its target rather than diving onto it.
see_also: [Splits, AirburstWeapon, Cluster, VeryHigh, ROT]
when_omitted:
  kind: value
  value: "no"
---

Only a projectile steered by the homing flight model is affected: one whose [`ROT`](/keys/rot/#scope-bullettype) is above zero.

An ordinary homing projectile follows the terrain while it is still some way out and then pitches straight at its target for the last stretch of the approach. An airburst projectile never takes that last stretch: the terrain-following branch stays engaged whatever distance is left, holding ten terrain levels of clearance above the ground ahead. Two further changes follow from it.

- The distance still to run is measured on the horizontal alone. Every other projectile counts a quarter of the height difference toward that distance; an airburst projectile discards the height entirely. It therefore counts as having arrived once it is over its target rather than once it has reached it.
- On arrival it is left where it is. A projectile that arrives while still in the air is normally snapped onto the target coordinate first, and one whose fuse trips close to its target is walked onto the victim. An airburst projectile skips both, so the blast goes off overhead.

It is also exempt from the check that forces a homing projectile which has stopped gaining on its target to detonate, since an airburst projectile is meant to sit above its target rather than close on it.

A projectile chasing an aircraft never follows the terrain in the first place; it flies straight at its quarry. On that shot the terrain clearance is what falls away. The horizontal distance measure, the detonation changes and the exemption from the stall check all still apply.

:::danger[This setting also switches splitting on, and a splitting projectile with no weapon crashes the game]
[`Splits`](/keys/splits/) takes its stored value from this one whenever the projectile's section does not set `Splits` of its own, so `Airburst=yes` on its own makes the projectile a splitting one. A splitting projectile reads the projectile of its [`AirburstWeapon`](/keys/airburstweapon/) without first checking that a weapon was named, so the game crashes the moment such a projectile detonates. Name an `AirburstWeapon` in the same section, or write `Splits=no` there.
:::

```ini title="rules.ini"
[MYCLUSTERMISSILE] ; a BulletType, registered by a weapon naming it as its Projectile
Image=MISLMLTI
ROT=4
Airburst=yes
Cluster=6                ; six bomblets
AirburstWeapon=MyBomblet ; a WeaponType, registered by being named here
```
