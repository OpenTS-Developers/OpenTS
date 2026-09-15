---
key: SpawnCutoff
summary: The spawn interval a smoke system stops emitting at.
see_also: [BehavesLike, SpawnFrames, Slowdown, SpawnTranslucencyCutoff, Lifetime]
when_omitted:
  kind: value
  value: "0.0"
---

The system's working interval, which starts at [`SpawnFrames`](/keys/spawnframes/) and grows by [`Slowdown`](/keys/slowdown/) each frame, is measured against this figure at the end of every frame. Once it is past, the system stops emitting; it stays on the map until the last particle it already made has died, and then goes. Only the `Smoke` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

Set below the opening interval, the test is met on the system's very first frame, so the plume emits at most one particle and is marked spent before that frame ends. A type that has neither setting gets exactly this. A plume that is meant to run needs a cutoff above `SpawnFrames`, and a non-zero `Slowdown` to reach it at all.

```ini title="rules.ini"
[MySmokeSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Smoke
HoldsWhat=MySmokePuff ; a ParticleType registered in [Particles]
SpawnFrames=10
Slowdown=.0025
SpawnCutoff=15.0 ; reached after 2000 frames
```

This is the only retirement condition a smoke system has of its own; the alternative is a positive [`Lifetime`](/keys/lifetime/), which cuts the plume off wherever its interval happens to have reached.
