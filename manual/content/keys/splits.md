---
key: Splits
summary: Breaks the projectile into a burst of bomblets when it detonates.
see_also: [Airburst, AirburstWeapon, Cluster, RetargetAccuracy]
when_omitted:
  kind: context-dependent
  note: "Whatever `Airburst` stands at in the same section. The read takes its default from that flag rather than from this key's own stored value, so `Airburst=yes` alone leaves the projectile splitting."
---

An ordinary projectile detonates [`Cluster`](/keys/cluster/) times, scattering each blast a cell or two from the point of impact. A splitting projectile applies it exactly once and then releases `Cluster` bomblets of its [`AirburstWeapon`](/keys/airburstweapon/) instead. Everything the bomblets use comes from that weapon: projectile, warhead, damage and range. They are launched at the weapon's speed, and a homing bomblet then works toward a hard-coded ceiling of 50 leptons per game frame rather than toward the weapon's own figure. [`RetargetAccuracy`](/keys/retargetaccuracy/) decides what each one is aimed at.

Three of the adjustments that move an ordinary detonation are skipped. The blast is not pulled back onto the point the fuse was aimed at, and neither of the close-range snaps onto an airborne or a ground target is applied. The walk onto a target it went off within 32 leptons of still happens, because that one is keyed to [`Airburst`](/keys/airburst/) rather than to this setting.

:::danger[A splitting projectile with no `AirburstWeapon` crashes the game]
The split reads the named weapon's projectile without first checking that a weapon was named, so the game crashes the moment such a projectile detonates. Because the stored value comes from [`Airburst`](/keys/airburst/), this reaches any projectile marked `Airburst=yes` that does not also name a weapon.
:::

:::caution[A later rules file resets this key]
The default handed to the read is the `Airburst` flag, not the value this key already holds. Every later rules layer that contains the projectile's section, for any key at all, overwrites this setting with whatever `Airburst` stands at. `Splits=no` on an `Airburst=yes` projectile must be repeated in every file that declares the section.
:::
