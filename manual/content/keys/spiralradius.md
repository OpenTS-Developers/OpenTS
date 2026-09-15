---
key: SpiralRadius
summary: The radius of the corkscrew a railgun trace winds around its beam, in leptons.
see_also: [BehavesLike, SpiralDeltaPerCoord, ParticlesPerCoord, PositionPerturbationCoefficient]
when_omitted:
  kind: value
  value: "25.0"
---

Every particle of the trace is placed this far out from the line between the muzzle and the target, at the angle [`SpiralDeltaPerCoord`](/keys/spiraldeltapercoord/) gives it, so the figure is the visible thickness of the corkscrew. It is in leptons, 256 to a cell, which puts the stock traces at `6` and `15`, a small fraction of a cell out from the beam. Only the `Railgun` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
SpiralRadius=15
SpiralDeltaPerCoord=.03
PositionPerturbationCoefficient=30
```

[`PositionPerturbationCoefficient`](/keys/positionperturbationcoefficient/) is applied on top and is measured in the same leptons, so a scatter set larger than this radius buries the coil in its own noise. The stock traces do exactly that, which is why they read as ragged streaks rather than as springs. Setting the scatter below this radius reverses it: the windings are then the larger displacement, and the trace reads as a corkscrew.
