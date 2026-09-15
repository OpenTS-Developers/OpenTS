---
key: PositionPerturbationCoefficient
summary: How far each railgun particle is displaced from its place on the spiral, in leptons.
see_also: [BehavesLike, SpiralRadius, MovementPerturbationCoefficient, VelocityPerturbationCoefficient]
when_omitted:
  kind: value
  value: "0.0"
---

Each of the three axes is offset independently by a fresh amount running from half the figure one way to half the other. A setting of `30` therefore moves a particle up to fifteen leptons along each axis from where the corkscrew put it. The offsets are in leptons, the same units as [`SpiralRadius`](/keys/spiralradius/), and are applied on top of it. Only the `Railgun` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
SpiralRadius=6
PositionPerturbationCoefficient=20 ; scatter of up to ten leptons, wider than the coil
```

This is the trace's placement scatter and nothing else. Where each particle then travels is [`MovementPerturbationCoefficient`](/keys/movementperturbationcoefficient/)'s business and how fast is [`VelocityPerturbationCoefficient`](/keys/velocityperturbationcoefficient/)'s, and a value here changes neither.
