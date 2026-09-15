---
key: IsLaser
summary: Draws a beam from the muzzle to the target as the weapon fires.
see_also: ["LaserInnerColor", "LaserOuterColor", "LaserOuterSpread", "LaserDuration", "IsBigLaser", "Charges"]
when_omitted:
  kind: value
  value: "no"
---

The beam is drawn on top of an ordinary shot rather than in place of one: the projectile is still created, still travels and still detonates, and the damage is entirely the projectile's. What the flag adds is two pieces of scenery drawn between the muzzle and the target's own aim point.

```ini title="rules.ini"
[MyObeliskRay] ; example WeaponType
IsLaser=yes
LaserInnerColor=255,0,0
LaserOuterColor=128,0,0
LaserOuterSpread=20,40,40
LaserDuration=15
Projectile=LLine ; a BulletType, registered by a weapon naming it as its Projectile
```

The first piece is the beam itself: a thin line in [`LaserInnerColor`](/keys/laserinnercolor/), flanked by a pair of glow lines in [`LaserOuterColor`](/keys/laseroutercolor/) jittered each frame by [`LaserOuterSpread`](/keys/laserouterspread/), lasting [`LaserDuration`](/keys/laserduration/) frames. The second is a screen glow spanning the same line, which brightens only the red of whatever lies beneath it whatever colors the beam was given. [`IsBigLaser=yes`](/keys/isbiglaser/) widens it.

A weapon with a barrel normally has its shot stepped forward the moment it is created so that it appears past the muzzle rather than inside it. A laser weapon skips that step, so its projectile starts at the barrel mounting. On a structure the beam also stops the turret animation dead and resets it to its first frame. That is how a [`Charges=yes`](/keys/charges/) turret drops out of its wind-up pose.

:::caution[The beam always comes from the first weapon slot]
Whichever slot fired, the colors, the spread, the duration and the wide-or-narrow glow are read from the weapon sitting in the object's first slot. A laser weapon in the second slot therefore draws itself in the first weapon's colors. A first-slot weapon that is not a laser at all supplies its own unset color and duration to the beam.
:::
