---
key: UseSparkParticles
summary: Sprays a particle system at the target alongside the shot.
see_also: ["AttachedParticleSystem", "UseFireParticles", "ROF"]
when_omitted:
  kind: value
  value: "no"
---

One system of the type named in [`AttachedParticleSystem=`](/keys/attachedparticlesystem/) is spawned at the muzzle and aimed at the target as the shot leaves. Unlike a fire weapon, a spark weapon keeps its projectile's damage: [`Damage=`](/keys/damage/#scope-weapontype) is delivered on impact as usual and the spray is added on top of it.

```ini title="rules.ini"
[MySparkGun] ; example WeaponType
UseSparkParticles=yes
AttachedParticleSystem=SparkSys ; a ParticleSystemType registered in [ParticleSystems]
ROF=30
```

A vehicle refuses the shot while it has anywhere to go. An infantry is held only by the ordinary bar on firing while actually in motion. Unlike a [`UseFireParticles=yes`](/keys/usefireparticles/) weapon, this one may still be fired by a soldier standing still with a destination pending. Neither of the object's weapons may fire again while the spray is alive. The reload delay is exactly [`ROF`](/keys/rof/), with the house's rate of fire bias, the burst gaps and the random padding all skipped. A structure with more than one round left waits a single frame instead, so only the effect's own lifetime paces it.

A weapon that also sets [`UseFireParticles=yes`](/keys/usefireparticles/) spawns the same particle system type twice over, once into the firer's fire slot and once into its spark slot, and must wait for both to burn out.

:::danger[A spark weapon with no particle system named crashes the game]
Nothing checks that [`AttachedParticleSystem=`](/keys/attachedparticlesystem/) resolved to anything before the spray is spawned, and the game stops the first time such a weapon fires.
:::
