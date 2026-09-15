---
key: LightSize
summary: How strong a light a particle system casts.
see_also: [BehavesLike, OneFrameLight, ParticleCap, SparkSpawnFrames]
when_omitted:
  kind: value
  value: "0"
---

The figure is a strength, not a distance: it scales the brightness the glow is drawn at, with `64` leaving it at its natural level. The stock systems' `21` lands at about a third of that and `25` at about two fifths. Zero or below casts no light at all.

```ini title="rules.ini"
[MyWeldingSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MyWeldingSpark ; a ParticleType registered in [Particles]
ParticleCap=25
SparkSpawnFrames=20
LightSize=25
OneFrameLight=true
```

There are two lights, and [`OneFrameLight`](/keys/oneframelight/) chooses between them. With the flag set, the glow is drawn afresh on every frame the system holds a particle, at this strength scaled by how full the system is against [`ParticleCap`](/keys/particlecap/). A system of any behavior can have one, and every detail setting draws it. Without the flag, a single glow is thrown at this strength when a `Spark` system's first frame bursts. It spreads and burns out over the next ten frames on its own, is drawn only at the highest detail setting, and only the `Spark` [behavior](/keys/behaveslike/#scope-particlesystemtype) casts it.

:::danger[A light size above 75 draws the glow from unrelated memory]
The glow is picked from a fixed run of 74 brightness ramps by scaling the ramp the light's age selects by this figure, and the pick is never tested against the run's length. From `76` up it can land past the last ramp, and the glow is then drawn through whatever value lies beyond the run rather than through a ramp. Keep the figure at `75` or below.
:::
