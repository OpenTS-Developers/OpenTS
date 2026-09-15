---
key: MovementPerturbationCoefficient
summary: How far each railgun particle's course is deflected from the spiral it was laid on.
see_also: [BehavesLike, VelocityPerturbationCoefficient, PositionPerturbationCoefficient, SpiralRadius]
when_omitted:
  kind: value
  value: "0.0"
---

A particle of the trace starts out traveling directly away from the beam, along the same line that placed it. Each of the three axes of that course is then deflected independently by an amount running from half this figure one way to half the other. The result is normalized back to a direction. The figure is therefore measured against a course of unit length: `.3` bends by up to fifteen hundredths of it on each axis. Only the `Railgun` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
MovementPerturbationCoefficient=.3
VelocityPerturbationCoefficient=.6
```

:::caution[The figure doubles as the floor on the speed wander]
[`VelocityPerturbationCoefficient`](/keys/velocityperturbationcoefficient/) bounds how far a particle's speed may drift from its type's own velocity, but only the upward bound is taken from it: the downward bound is this figure negated. Raising the deflection therefore lets the trace's particles run further below their type's speed, and a system that sets no deflection at all can gain speed from the wander but never lose it.
:::
