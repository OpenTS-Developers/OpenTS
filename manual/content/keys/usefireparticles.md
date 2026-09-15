---
key: UseFireParticles
summary: Attacks with a stream of fire particles instead of the projectile's damage.
see_also: ["AttachedParticleSystem", "UseSparkParticles", "ROF"]
when_omitted:
  kind: value
  value: "no"
---

The projectile is still created and still flies, but its firepower is forced to nothing, so [`Damage=`](/keys/damage/#scope-weapontype) is never delivered. Everything the weapon does is done by the particle system named in [`AttachedParticleSystem=`](/keys/attachedparticlesystem/), which is spawned at the muzzle and aimed at the target as the shot leaves.

```ini title="rules.ini"
[MyFlamer] ; example WeaponType
UseFireParticles=yes
AttachedParticleSystem=FireStreamSys ; a ParticleSystemType registered in [ParticleSystems]
ROF=30
```

The firer must stand still, and more than still: a vehicle and an infantry both refuse the shot while a destination is pending, whether or not they have started moving toward it. Neither of the object's weapons may fire again while the stream is alive. The reload delay is then exactly [`ROF`](/keys/rof/): the house's rate of fire bias, the burst gaps and the random padding are all skipped. The stream's own lifetime and `ROF` together set the firing rate. A structure with more than one round left waits a single frame instead, so only the effect's own lifetime paces it.

Assigning the object a target it cannot reach kills the stream at once: the system is removed the moment a new target is set that is out of range. Clearing the target altogether is not one of these moments; the removal check passes over an empty target and the stream stays.

:::danger[A fire weapon with no particle system named crashes the game]
Nothing checks that [`AttachedParticleSystem=`](/keys/attachedparticlesystem/) resolved to anything before the stream is spawned, and the game stops the first time such a weapon fires.
:::
