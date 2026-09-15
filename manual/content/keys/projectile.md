---
key: Projectile
summary: The BulletType the weapon launches.
see_also: ["Warhead", "Speed", "ProjectileRange", "Range", "system:target-selection"]
when_omitted:
  kind: value
  value: none
---

The projectile decides how the shot travels and what it may be aimed at, while the weapon decides how hard it hits and how often. Whether the shot homes, arcs, bounces, burns fuel, or is drawn at all belongs to the named BulletType. So do the [`AA`](/keys/aa/), [`AG`](/keys/ag/) and [`AV`](/keys/av/) flags that decide which kinds of object the weapon may be fired at, and through them [what an object scanning for a target asks for](/systems/target-selection/#what-each-kind-of-object-considers).

```ini title="rules.ini"
[MyMissileGun] ; example WeaponType
Projectile=MyRocket ; a BulletType, registered by a weapon naming it as its Projectile
Warhead=AP         ; a WarheadType registered in [Warheads]
Damage=60
Range=8

[MyRocket] ; example BulletType
ROT=20
AA=yes
```

The choice also decides whether the weapon's own [`Speed=`](/keys/speed/#scope-weapontype) survives: a projectile that does not home has the weapon's launch speed recomputed from its [`Range=`](/keys/range/) once the rules have been read.

A name the game does not already know is registered as a new projectile of that name rather than rejected. A misspelling therefore produces a projectile with none of the intended settings instead of an error. Writing the key with nothing after the `=` is a different thing again: the read finds no value and keeps whatever an earlier rules file set.

:::danger[A weapon with no projectile crashes the game]
`Projectile=none`, and leaving the key unwritten, both leave the weapon with nothing to fire, and none of the paths that ask about a shot check for that first. Merely considering a target reads the projectile's anti-air flag. Measuring range reads its arcing flag. Filling in what an object may scan for reads its anti-vehicle flag. Rating a structure's base defenses reads its anti-ground flag. Any of them stops the game. A structure marked [`IsBaseDefense=yes`](/keys/isbasedefense/) even has its projectile's anti-air flag read while the rules are still being loaded, so the game stops before the scenario opens. Every weapon that an object can hold needs a projectile named, whether or not it is meant to be fired.
:::
