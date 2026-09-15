---
key: OneFrameLight
summary: Whether a system's light is redrawn every frame at a brightness that follows how full of particles it is.
see_also: [LightSize, ParticleCap, BehavesLike]
when_omitted:
  kind: value
  value: "no"
---

With the flag set, the system draws its own glow on every frame it holds at least one particle. The glow's strength is [`LightSize`](/keys/lightsize/) scaled by the particles it holds against [`ParticleCap`](/keys/particlecap/). The light comes up as the system fills and dies away with the last of its sparks. Without it, a `Spark` system throws one glow at the start that spreads and burns out on its own, and no other [behavior](/keys/behaveslike/#scope-particlesystemtype) lights anything.

```ini title="rules.ini"
[MyWeldingSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MyWeldingSpark ; a ParticleType registered in [Particles]
ParticleCap=25
SparkSpawnFrames=20
LightSize=25
OneFrameLight=true
```

The two are alternatives rather than additions: setting the flag suppresses the thrown glow entirely, so a spark shower has one light or the other and never both. The flag is also the one part of the lighting a system of any behavior can use. A smoke plume or a gas cloud that sets it and a `LightSize` glows, where the same type without it stays dark.

A spark system's light flickers because the spread it is drawn at steps three either way from frame to frame, on about three frames in five. The spread is held between 17 and 41. A system of any other behavior holds that spread at a fixed 29, so its light is steady and follows only the particle count. The spread is a radius in pixels: the glow always covers the same patch of screen, about 256 pixels across, and the figure shapes how its brightness falls away.
