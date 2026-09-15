---
key: Lifetime
summary: How many frames a particle system runs before it is retired.
see_also: [BehavesLike, SpawnCutoff, SparkSpawnFrames]
when_omitted:
  kind: value
  value: "-1"
---

The count runs down one a frame from the moment the system appears and is the one retirement condition every [behavior](/keys/behaveslike/#scope-particlesystemtype) shares. The others each have at most one of their own, and `Gas`, `WeakGas` and `Web` have none. Reaching the end stops the system on that frame, whatever its behavior. Its update routine never runs again, and any particles it still holds stop where they are instead of aging on. Their aging and drift run only inside that routine, so nothing else ever moves or removes them: a plume caught this way sits on the map for the rest of the scenario. A smoke system that passes [SpawnCutoff](/keys/spawncutoff/) or a spark shower whose [SparkSpawnFrames](/keys/sparkspawnframes/) have run out stops emitting but keeps aging its particles. It leaves the map only when the last of them has died.

```ini title="rules.ini"
[MyGasPuffSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=WeakGas
HoldsWhat=MyWeakGas ; a ParticleType registered in [Particles]
Lifetime=3
```

:::caution[Zero and below mean forever, not at once]
The count is tested for exactly zero after each frame's decrement, so a value of `0` steps straight past the test to `-1` and every value below it moves further away. Only a positive figure ever retires a system this way. A gas or web type has no other retirement, so a system of one of those left at or below zero runs for the rest of the scenario. A smoke type has its cutoff and a spark type its burst count, when that count is positive, to retire it instead.
:::
