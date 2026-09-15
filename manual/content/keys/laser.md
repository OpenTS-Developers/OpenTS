---
key: Laser
summary: Whether a railgun system draws a beam alongside its spiral of particles.
see_also: [BehavesLike, LaserColor, IsRailgun, IsLaser, AttachedParticleSystem]
when_omitted:
  kind: value
  value: "no"
---

The beam is created once, on the same frame the trace is laid, and runs the length of the trace. On a railgun weapon that is from the firer's muzzle to the center of what it fired at, unless the beam trips a collapsible cliff. It is a single line in [`LaserColor`](/keys/lasercolor/) with no outer glow, it lasts ten frames, and it fades from half intensity to nothing across them. The lowest detail setting draws it as a flat line of that color instead, with no fade. Above that setting, a color channel of zero is left unpainted rather than drawn black.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
Laser=yes
LaserColor=25,20,255
```

Only the `Railgun` [behavior](/keys/behaveslike/#scope-particlesystemtype) reads the flag, and the beam is independent of the particles beside it: a trace that lays no particles still draws it. A system that sets the flag and leaves `LaserColor` unset draws nothing visible above the lowest detail setting, because every one of its channels is left at zero. At the lowest setting the same system paints a black line.

This is not the beam a laser weapon fires. That one belongs to [`IsLaser=yes`](/keys/islaser/) on the WeaponType, takes its colors from the weapon, and reaches nothing here.
