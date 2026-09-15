---
key: SpawnFrames
summary: The starting interval, in game frames, between a smoke or fire system's particle spawns.
see_also: [BehavesLike, Slowdown, SpawnCutoff, SpawnTranslucencyCutoff, NaturalParticleSystem]
when_omitted:
  kind: value
  value: "1"
---

A system emits on the frames whose number divides by this interval, so a smoke plume set to `10` puts out a particle roughly once every two thirds of a second. Only the `Smoke` and `Fire` [behaviors](/keys/behaveslike/#scope-particlesystemtype) reach it.

```ini title="rules.ini"
[MySmokeSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Smoke
HoldsWhat=MySmokePuff ; a ParticleType registered in [Particles]
SpawnFrames=10
Slowdown=.0025
SpawnCutoff=15.0
```

A smoke system keeps a working interval of its own that starts here and is stretched by [`Slowdown`](/keys/slowdown/) every frame, so the setting is only the plume's opening rate. Two other settings are read against that stretched interval rather than against this one: [`SpawnCutoff`](/keys/spawncutoff/) retires the system once the interval passes it, and [`SpawnTranslucencyCutoff`](/keys/spawntranslucencycutoff/) starts thinning the new particles. This figure is also the mark the plume's slowing is measured from: each frame the working interval has grown past it subtracts `0.35` leptons a frame from the speed a new particle is created with. That subtraction bottoms out at a speed of `2`, so no particle is ever created slower than that. A plume set to `10` and retired at `15` therefore puts out its last particles `1.75` slower than its first.

A fire system holds the interval fixed at this figure; `Slowdown` never reaches it. It also emits on one more schedule: while its firer has a target and is still turning, a particle goes out every third frame, so a stream thickens as the shooter comes around.

A structure that is knocked to half strength stretches its [`NaturalParticleSystem`](/keys/naturalparticlesystem/)'s working interval by half again on the spot, so the plume immediately thins and reaches its cutoff sooner than an undamaged one would.

:::danger[`SpawnFrames=0` crashes the game]
Both routines divide the frame counter by the interval without checking it first. A `Smoke` or `Fire` system set to zero crashes the game on the first frame one of its systems runs.
:::
