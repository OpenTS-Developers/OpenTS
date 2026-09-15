---
key: SparkSpawnFrames
summary: How many frames a spark system goes on throwing bursts for.
see_also: [BehavesLike, ParticleCap, SpawnSparkPercentage, LightSize, Lifetime]
when_omitted:
  kind: value
  value: "0"
---

The count runs down one a frame, and the system stops emitting and begins to clear away when it reaches the end. Its last frame always throws a burst whatever [`SpawnSparkPercentage`](/keys/spawnsparkpercentage/) says, so a shower set to `1` is a single guaranteed burst and nothing more. Only the `Spark` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MyWeldingSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MyWeldingSpark ; a ParticleType registered in [Particles]
ParticleCap=25
SparkSpawnFrames=20
SpawnSparkPercentage=.4
LightSize=25
OneFrameLight=true
```

Only the system's first frame can cast the aging glow described under [`LightSize`](/keys/lightsize/), and only when that frame happens to throw a burst, so a longer shower is not a longer light. The steady one-frame light the example sets instead is redrawn for as long as the system still holds a particle, which outlasts the count.

:::caution[A spark system left at zero never clears itself away]
The burst logic is skipped while the count is not above zero, so such a system throws nothing and never reaches the condition that retires it. Without a positive [`Lifetime`](/keys/lifetime/) it sits on the map, doing nothing, for the rest of the scenario.
:::
