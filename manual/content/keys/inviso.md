---
key: Inviso
summary: Removes the flight entirely; the projectile appears at its target the moment it is fired and is never drawn.
see_also: [Image, IgnoresFirestorm]
when_omitted:
  kind: value
  value: "no"
---

The projectile is placed at its target coordinate as it is launched, with its speed set to zero, and detonates there. Nothing is drawn on the way, and the drawing routine returns before it reads a frame.

An active firestorm wall standing on the line between firer and target is the one thing that stops it. The projectile is placed at the wall, consumed by it and removed without detonating. The shot deals no damage at all: not to the wall, not to anything else. Walls belonging to the firer's own house let it through; [`IgnoresFirestorm=yes`](/keys/ignoresfirestorm/) does not, because that setting is only read by a projectile that is actually in flight.

Because the impact would otherwise land on exactly the same spot every time, the explosion animation is thrown 32 leptons (an eighth of a cell) from the detonation point in a random direction. The damage itself is applied at the detonation point.

An aircraft whose first weapon fires an invisible projectile makes no strafing runs.
