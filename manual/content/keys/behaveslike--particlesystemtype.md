---
key: BehavesLike
scope: particlesystemtype
label: Particle system behavior
see_also: [HoldsWhat, Lifetime, Particle, DamageParticleSystems, AttachedParticleSystem]
when_omitted:
  kind: value
  value: none
  note: No name is supplied rather than the behavior already in force, so a later file that contains the section without this key leaves the type with no behavior at all.
---

One of seven names, and it picks which update routine drives every system built from the type. The match ignores letter case, so `spark` names the same behavior as `Spark`. Each routine reads a different part of the section, so most of the settings alongside it are read under one behavior and passed over in silence under the rest. [What each system behavior reads](/systems/particle-systems/#what-each-system-behavior-reads) gives the division setting by setting.

| Value | What a system of the type does each frame |
| --- | --- |
| `Smoke` | Emits a particle on an interval that lengthens as the plume tires, splits an expiring particle into a pair of its successors thrown out to either side, and rides along with a source that is not a building. |
| `Fire` | Aims along the firer's body facing while that facing is still turning, and launches a particle toward a point that swings in and out along the line of fire. |
| `Spark` | Throws a burst of particles on a roll, each flung along a randomly deflected direction, for a fixed run of frames. |
| `Railgun` | Lays one corkscrew of particles along the line from the firer to the target on its first frame, draws the beam, and afterwards only ages what it laid. |
| `Gas`, `WeakGas`, `Web` | Drifts the particles it holds and replaces each expiring one with that particle's own successor, in place. It emits nothing of its own, so everything it holds arrives from whatever asked it for a particle. |

`Gas`, `WeakGas` and `Web` drive their particles identically, so nothing a system of one of those three does with its particles distinguishes it from the other two. They part company only where a setting sorts systems by this value: a warhead's [`Particle`](/keys/particle/) tests for `Gas` and for nothing else. The same three names on a [particle's own section](/keys/behaveslike/#scope-particletype) do differ from one another, but that is a separate setting.

```ini title="rules.ini"
[MyRailgunSys] ; a ParticleSystemType registered in [ParticleSystems]
BehavesLike=Railgun
HoldsWhat=MyRailgunPart ; a ParticleType registered in [Particles]
SpiralRadius=15
ParticlesPerCoord=.15
Laser=yes
LaserColor=25,20,255
```

Two settings elsewhere read this value to choose between systems rather than to run them. [`DamageParticleSystems`](/keys/damageparticlesystems/) is one pool that is split into its `Spark` entries and its `Smoke` entries, and an entry with any other behavior is never drawn from it. A warhead's [`Particle`](/keys/particle/) is tested for `Gas` before the blast decides whether to build a system at all: a gas type releases its particle into the scenario's shared cloud instead.

:::caution[A system with no behavior never goes away on its own]
An unrecognized name leaves the type without a behavior, and so does omitting the key. No routine runs for such a type: it emits nothing, and none of the retirement conditions the behaviors have can be reached. Such a system goes only when something outside it says so: a positive [`Lifetime`](/keys/lifetime/), or the loss of the object it was attached to. One created with neither stays on the map for the rest of the scenario.
:::
