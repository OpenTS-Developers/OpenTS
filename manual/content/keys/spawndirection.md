---
key: SpawnDirection
summary: The direction a spark burst is thrown in.
see_also: [BehavesLike, ParticleCap, SparkSpawnFrames, DefaultFirestormExplosionSystem]
when_omitted:
  kind: value
  value: "0,0,0"
---

Three comma-separated components, added to each spark's own randomly picked velocity before the sum is normalized and scaled back to the speed it had. The setting therefore bends a shower toward a direction without changing how fast its sparks travel, and how strongly it bends depends on its size against the particle type's own velocity ranges. Left at zero, the sparks scatter as those ranges alone dictate.

```ini title="rules.ini"
[MyVentSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MySpark ; a ParticleType registered in [Particles]
ParticleCap=25
SparkSpawnFrames=20
SpawnDirection=0,0,60 ; sparks thrown upward
```

Only the `Spark` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it, and one caller sets it aside: each of the seven to nine systems thrown up when a vehicle or aircraft is destroyed by the firestorm warhead — the type named by [`DefaultFirestormExplosionSystem`](/keys/defaultfirestormexplosionsystem/) — picks a bias of its own at random for the whole burst instead, which is what makes those explosions differ from one another.

:::note[A value with fewer than three components reads as the default]
`SpawnDirection=0` or `SpawnDirection=0,60` is short of the three components a vector needs, so the key reads as its default and the debug log records the line; [INI syntax](/formats/ini-syntax/#malformed-values) has the rule.
:::
