---
key: SpawnSparkPercentage
summary: The chance that a spark system throws a burst on any one frame.
see_also: [BehavesLike, SparkSpawnFrames, ParticleCap]
when_omitted:
  kind: value
  value: "0.0"
---

A fraction from `0` to `1`, rolled once per frame while the system still has [`SparkSpawnFrames`](/keys/sparkspawnframes/) left. At `.4` about two frames in five throw a burst, which is what makes a welding shower stutter rather than pour. Each throw scatters one burst, its size set against [`ParticleCap`](/keys/particlecap/). Only the `Spark` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MySparkSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MySpark ; a ParticleType registered in [Particles]
ParticleCap=12
SparkSpawnFrames=20
SpawnSparkPercentage=.4 ; roughly two frames in five throw
```

The roll is skipped on the system's final spark frame, which throws unconditionally, so a system whose rolls all fail still produces that one burst before it clears away. A figure of `1` or above throws on every frame.
