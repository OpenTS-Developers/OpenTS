---
key: HoldsWhat
summary: The ParticleType every particle a system creates is made from.
see_also: [BehavesLike, NextParticle, Particle]
when_omitted:
  kind: value
  value: ""
  note: An empty name is registered as a ParticleType of its own rather than resolving to nothing, so the type holds a blank particle with every built-in value; a later file that contains the section without this key puts that blank particle back.
---

```ini title="rules.ini"
[MySparkSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Spark
HoldsWhat=MySpark ; a ParticleType registered in [Particles]
ParticleCap=12
SparkSpawnFrames=1
SpawnSparkPercentage=1
```

Every behavior that creates a particle creates one of this type, and it is the only particle a system makes for itself. A particle's own [`NextParticle`](/keys/nextparticle/) governs what it turns into afterwards, so a gas cloud or a smoke column may end up holding types this key never names.

A name no particle declares is registered as a new particle type rather than refused, and a misspelling therefore produces a system holding a particle with no artwork, no damage and a one-frame life instead of an error. That is also what an omitted key resolves to: the blank name is registered as a particle in its own right, so the system holds something invisible rather than holding nothing. Every type in the rules that omits the key ends up sharing that one blank particle.

:::caution[A gas system named by a warhead never supplies its own particle]
A warhead whose [`Particle`](/keys/particle/) names a system with [`BehavesLike=Gas`](/keys/behaveslike/#scope-particlesystemtype) releases into the scenario's shared gas cloud rather than building a system of its own. The particle released is the shared cloud's rather than the one named here. The setting on such a type is never reached by that path.
:::

:::danger[`HoldsWhat=<none>` crashes a spark or a railgun system]
`<none>` is the one value that resolves to no particle. A smoke or fire system checks for it and quietly emits nothing, and a gas or web system never creates a held particle in the first place. A `Spark` system reaches past the start of the particle list for a type that is not there, and a `Railgun` system lays its corkscrew through a particle it never created. The spark system crashes the first frame it throws a burst; the railgun system crashes the first time it lays a trace with at least one particle in it. Leave the key out, or name a particle.
:::
