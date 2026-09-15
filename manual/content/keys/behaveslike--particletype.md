---
key: BehavesLike
scope: particletype
label: Particle behavior
see_also: ["MaxEC", "NextParticle", "Velocity", "WindEffect", "Warhead"]
when_omitted:
  kind: value
  value: none
  note: No name is supplied rather than the behavior already in force, so a later file that contains the section without this key leaves the type with no behavior at all.
---

Seven names are recognized. Each selects one routine that ages a particle of the type, and six of them also move it: `Gas`, `WeakGas`, `Smoke` and `Fire` in a second routine, `Spark` and `Railgun` inside the aging routine itself. `Web` holds its position, and nothing else in the section changes which routines run.

| Value | What a particle of the type does |
| --- | --- |
| `Gas` | Settles toward the ground and wanders sideways, glancing off terrain, bridge decks, walls and buildings, and applies its damage and warhead to whatever shares its cell. |
| `WeakGas` | Moves and ages exactly as `Gas` does, but never applies damage. |
| `Smoke` | Climbs at its own speed, wanders a little to either side, and slows until it is rising at 3 leptons per frame. |
| `Fire` | Coasts along the line it was fired on, thins as its states advance, scorches everything sharing its cell except the object its system is attached to, and dies where the ground rises to meet it. |
| `Spark` | Falls under gravity and dies where it strikes ground, a bridge deck or a wall. Drawn as a single lit pixel rather than from artwork. |
| `Railgun` | Rides its firing direction at a speed that wanders from frame to frame. Drawn as a single lit pixel rather than from artwork. |
| `Web` | Holds its position and delivers its [warhead](/keys/warhead/#scope-particletype), at zero damage, to everything in its cell every frame. |

A name outside that set leaves the type with no behavior: its particles hold still, draw frame zero of their artwork, and do nothing but count [`MaxEC`](/keys/maxec/) down to expiry.

The behavior a particle's own section names is separate from the one its holding system names. [The system's behavior](/keys/behaveslike/#scope-particlesystemtype) decides how particles are emitted and aimed and whether [`NextParticle`](/keys/nextparticle/) successors are created at all; this one decides what each particle does once it exists.
