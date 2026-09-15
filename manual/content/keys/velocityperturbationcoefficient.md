---
key: VelocityPerturbationCoefficient
summary: How far a railgun particle's speed may run above its own type's velocity.
see_also: [BehavesLike, MovementPerturbationCoefficient, PositionPerturbationCoefficient, Velocity]
when_omitted:
  kind: value
  value: "0.0"
---

The trace holds a running speed offset, one figure rather than a vector on the axes, added to each particle's [`Velocity`](/keys/velocity/) as it is laid. The particle keeps the direction the system gave it; the offset only makes it travel that course faster or slower. Each step adds a fresh random draw to the offset already in hand and scales the sum by half this figure. The offset stays bounded, so neighboring particles differ little and the trace ripples along its length rather than jumping about. Only the `Railgun` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads it.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
MovementPerturbationCoefficient=.3
VelocityPerturbationCoefficient=.6
```

This figure bounds the offset from above; [`MovementPerturbationCoefficient`](/keys/movementperturbationcoefficient/) negated bounds it from below, so the wander is not symmetric unless the two have the same value. The ceiling is further away than it looks: each step scales the running offset down along with the fresh draw, so no setting below `1.5` can work the offset up to it. The offset is absolute rather than proportional. With the stock `.6` it stays under `.22` however long the trace runs, so the stock railgun particles, at `.4` and `.3`, wobble by a fraction of their own speed, never a multiple of it.

At zero the offset never moves, and every particle is laid at its type's velocity exactly. The exception is a negative [`MovementPerturbationCoefficient`](/keys/movementperturbationcoefficient/): the floor is that figure negated, so a negative one lifts the offset off zero. Once the negated figure passes this one, it pins every particle at exactly that boost above its type's speed.
