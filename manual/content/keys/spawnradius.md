---
key: SpawnRadius
summary: How far a smoke system scatters its new particles from its own position, in leptons.
see_also: [BehavesLike, SpawnFrames, NaturalParticleLocation, DamageSmokeOffset]
when_omitted:
  kind: value
  value: "0"
---

Each particle the system emits is displaced on both horizontal axes by an independent amount running up to the full radius either way, so the column rises from a patch rather than from a point. The figure is in leptons, 256 to a cell, so the stock plumes, at `3` to `10`, spread across a small fraction of the cell they stand on. Every particle is also lifted a fixed ten leptons above the system, which no setting changes.

```ini title="rules.ini"
[MySmokeSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Smoke
HoldsWhat=MySmokePuff ; a ParticleType registered in [Particles]
SpawnFrames=10
SpawnRadius=10 ; particles appear within ten leptons of the plume's base
```

Only the `Smoke` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it. Where the plume as a whole sits is not this setting's business: a structure's own plume is placed by [`NaturalParticleLocation`](/keys/naturalparticlelocation/) and a damage plume by [`DamageSmokeOffset`](/keys/damagesmokeoffset/).

:::danger[`SpawnRadius=-1` crashes the game]
The radius is widened by one and then divided into a random number, so exactly `-1` divides by zero and crashes the game the first frame the plume emits.
:::
