---
key: SpawnTranslucencyCutoff
summary: The spawn interval past which a smoke system creates its particles a step more faded.
see_also: [BehavesLike, SpawnFrames, Slowdown, SpawnCutoff, Translucency]
when_omitted:
  kind: value
  value: "0.0"
---

Once the system's working interval has grown past this figure, each particle it emits from then on is created a step of 25 beyond the [`Translucency`](/keys/translucency/#scope-particletype) its own type calls for. The plume visibly fades before it stops. Set it between [`SpawnFrames`](/keys/spawnframes/) and [`SpawnCutoff`](/keys/spawncutoff/) to put the fade in the plume's last stretch; set it below the opening interval and every particle the plume ever emits is faded.

```ini title="rules.ini"
[MySmokeSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Smoke
HoldsWhat=MySmokePuff ; a ParticleType registered in [Particles]
SpawnFrames=10
Slowdown=.0025
SpawnTranslucencyCutoff=13.0 ; fade begins after 1200 frames
SpawnCutoff=15.0
```

Only the `Smoke` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it, and only for the particles the system itself emits. The pair of successors a smoke particle splits into on its death take their parent's fade plus a further step of their own, five times in six, whatever this setting says.

The step is only worth setting where the particle type's own figure leaves room for it. At the highest detail setting three fade levels are drawn and no others. A puff authored at `25` is drawn at `50` once the plume thins. One authored off those levels is drawn solid to begin with, and the step only fades it where it lands exactly on `25` or `50` or reaches `75`. `75` is the cap: anything past it draws at the same blend as `75` itself, so the step never fades a particle to nothing. Below that setting no particle is faded at all.
