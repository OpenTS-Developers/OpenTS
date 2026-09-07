---
key: AirburstWeapon
summary: The WeaponType supplying the bomblets a splitting projectile releases.
see_also: [Splits, Airburst, Cluster, RetargetAccuracy]
when_omitted:
  kind: value
  value: none
---

Only a [`Splits=yes`](/keys/splits/) projectile reads the weapon, and a projectile left splitting by [`Airburst=yes`](/keys/airburst/) counts as one. Naming a weapon here changes nothing on a projectile that does not split.

Each bomblet carries the named weapon's own [`Projectile`](/keys/projectile/), [`Warhead`](/keys/warhead/#scope-weapontype) and [`Range`](/keys/range/), and ten times its [`Damage`](/keys/damage/#scope-weapontype). None of the carrier's own figures reach a bomblet. The bomblets are launched from the point the carrier detonated at, pointed straight down at the named weapon's [`Speed`](/keys/speed/#scope-weapontype), and whoever fired the carrier is credited with what they kill. Only the launch is at that speed: a homing bomblet then works toward a hard-coded ceiling of 50 leptons per game frame rather than toward its weapon's own figure.

The name is resolved by registering a weapon of that name if one is not already known, so a name that matches no weapon section produces an empty weapon rather than an error. `none` and `<none>` are the two names not resolved that way; both leave the setting holding no weapon at all.

:::danger[A splitting projectile with no usable weapon here crashes the game]
The split reads the weapon and its projectile without testing either, so a `Splits=yes` projectile crashes the game the moment it detonates if this key is absent, set to `none` or `<none>`, misspelled, or naming a weapon that has no `Projectile`. Both placeholders resolve to no weapon at all and crash exactly as an absent key does, and a misspelling registers an empty weapon, which has no projectile. So does a correctly spelled weapon this key alone names, because it is registered after the weapon sections are read; give it a line in the rules [`[Weapons]` list](/formats/rules-registries/) to register it in time.
:::
