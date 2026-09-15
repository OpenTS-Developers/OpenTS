---
key: Slowdown
summary: How much a smoke system's spawn interval lengthens every frame.
see_also: [BehavesLike, SpawnFrames, SpawnCutoff, SpawnTranslucencyCutoff, WindEffect]
when_omitted:
  kind: value
  value: "0.0"
---

The amount is added to the system's working interval on every frame it runs, not on every particle it emits, so the plume pours fastest when it appears and thins steadily from there. Only the `Smoke` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it, and the puffs' own sideways drift is governed by the particle type's [WindEffect](/keys/windeffect/).

```ini title="rules.ini"
[MySmokeSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Smoke
HoldsWhat=MySmokePuff ; a ParticleType registered in [Particles]
SpawnFrames=10
Slowdown=.0025
SpawnTranslucencyCutoff=13.0
SpawnCutoff=15.0
```

The rate sets the plume's whole life span, because it is what takes the interval up to [`SpawnCutoff`](/keys/spawncutoff/). The system above stretches from `10` to `15` at `.0025` a frame, which is 2000 frames (a little over two minutes), and crosses [`SpawnTranslucencyCutoff`](/keys/spawntranslucencycutoff/) after 1200 of them. Left at zero the interval never moves, so the plume either runs until something else removes it or, where the cutoff is lower than [`SpawnFrames`](/keys/spawnframes/), is retired on its first frame.

:::danger[A negative value can crash the game]
Nothing stops the interval shrinking. A negative amount walks it back down toward zero. The frame counter is divided by the whole-number part of it, so a plume whose interval is dragged below `1` crashes the game on the next frame it tests. It takes a [`SpawnCutoff`](/keys/spawncutoff/) at or above the opening interval to get there: with a lower cutoff the system is marked spent on its first frame and stops testing the interval at all.
:::
