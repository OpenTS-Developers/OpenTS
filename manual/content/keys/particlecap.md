---
key: ParticleCap
summary: The size of a spark system's burst, and the fullness a one-frame light's brightness is measured against.
see_also: [BehavesLike, SparkSpawnFrames, SpawnSparkPercentage, OneFrameLight, LightSize]
when_omitted:
  kind: value
  value: "50"
---

Each burst a `Spark` system throws holds at least half this figure and never quite the whole of it. The count is half the figure plus a random amount up to one short of that half, so `12` gives a burst of six to eleven. The halves are whole numbers, so an odd figure loses its odd unit: `13` gives the same six-to-eleven burst as `12`. A [`OneFrameLight`](/keys/oneframelight/) system of any behavior reads it a second way: it divides the particles it currently holds by this figure and draws its glow at that fraction of [`LightSize`](/keys/lightsize/). The fraction is held between four tenths and the whole, so the light waxes as the system fills and never falls below the floor.

```ini title="rules.ini"
[MySparkSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MySpark ; a ParticleType registered in [Particles]
ParticleCap=12 ; bursts of six to eleven
SparkSpawnFrames=1
SpawnSparkPercentage=1
```

Nothing enforces the figure as a limit. A system may hold any number of particles at once, and a `Smoke`, `Fire`, `Gas`, `Web` or `Railgun` system is not counted against it at all.

:::danger[A ParticleCap of 1, 0 or -1 crashes a spark system]
The burst size divides a random number by half the figure, and each of those three halves to zero. A `Spark` system with one of them crashes the game the first time it throws a burst.
:::
