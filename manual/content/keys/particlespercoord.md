---
key: ParticlesPerCoord
summary: How many particles a railgun trace lays per lepton of beam length.
see_also: [BehavesLike, SpiralRadius, SpiralDeltaPerCoord, Laser, AttachedParticleSystem]
when_omitted:
  kind: value
  value: ".1"
---

The trace is laid in one pass on the system's first frame, and the count is the straight-line distance from the firer to the far end of the beam, multiplied by this figure and truncated. Distance is in leptons, 256 to a cell, so `.1` puts about 26 particles in every cell the beam crosses and a shot across five cells lays around 128 of them. Only the `Railgun` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
ParticlesPerCoord=.15
SpiralRadius=15
SpiralDeltaPerCoord=.03
```

The particles are spread evenly along the beam whatever the count, so the figure controls the trace's density rather than its length. A low value leaves a dotted corkscrew and a high one a solid rope. At zero or below no particle is laid at all, and the system does nothing but draw the beam [`Laser=yes`](/keys/laser/) asks for and retire.
