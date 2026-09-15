---
key: IsRailgun
summary: Punches a damaging line through the world from the muzzle to the target as the weapon fires.
see_also: ["AmbientDamage", "AttachedParticleSystem", "ROF", "Warhead"]
when_omitted:
  kind: value
  value: "no"
---

The beam is worked out the instant the shot goes off, before the projectile has traveled anywhere. The line from the muzzle to the target's center is walked in 50 steps, and every cell it passes into is examined. An object standing there is damaged when its center lies within [`[CombatDamage] RailgunDamageRadius`](/keys/railgundamageradius/) of the line. A structure in a crossed cell is never measured against that distance and always counts. The object aimed at is damaged whether or not the line actually crossed it. Each victim takes [`AmbientDamage`](/keys/ambientdamage/) through the weapon's [`Warhead=`](/keys/warhead/#scope-weapontype), once, and the firing object is skipped.

```ini title="rules.ini"
[MyRailgun] ; example WeaponType
IsRailgun=yes
Projectile=MyBolt ; a BulletType, registered by a weapon naming it as its Projectile
AmbientDamage=200 ; the beam's damage
Damage=0          ; the projectile has none
AttachedParticleSystem=LargeRailgunSys ; a ParticleSystemType registered in [ParticleSystems]
ROF=60
```

The walk stops at the first point where the ground stands higher than the beam, so a hill between the two ends of the shot cuts it short there. A destroyable cliff at that point may be brought down, at the chance [`[CombatDamage] CollapseChance`](/keys/collapsechance/) sets. The beam's own settled point is where the particle system is drawn to.

Unlike a sonic weapon, a railgun does not give up its projectile's damage. [`Damage=`](/keys/damage/#scope-weapontype) is still held by the shot and still delivered on impact, so a weapon meant to do its work through the beam alone must set it to nothing.

The particle system named by [`AttachedParticleSystem=`](/keys/attachedparticlesystem/) is spawned along the beam, and neither of the object's weapons may fire again until it has burned out. While one is alive the reload delay is exactly [`ROF`](/keys/rof/): the house's rate of fire bias, the burst gaps and the random padding are all skipped. The effect's own lifetime and `ROF` together decide the firing rate. A structure with more than one round left waits a single frame instead, so only the effect's own lifetime paces it.

:::danger[A railgun with no particle system named crashes the game]
Nothing checks that [`AttachedParticleSystem=`](/keys/attachedparticlesystem/) resolved to anything before the system is spawned, and the game stops the first time such a weapon fires. The beam is walked first, so its damage is dealt to everything under the line and the game then stops.
:::
