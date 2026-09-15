---
key: TurretSpins
summary: Makes a vehicle's turret rotate continuously instead of holding an aim.
see_also: ["Turret"]
when_omitted:
  kind: value
  value: "no"
---

Only a vehicle reads the flag. Once per frame its turret facing is snapped forward by 8 of the 256 directions, one thirty-second of a circle. The turret comes round once every 32 frames, a little over two seconds, at a rate no [`ROT`](/keys/rot/#scope-aircrafttype) can change.

The snap replaces the facing outright rather than setting a direction to turn toward. An aim given to the turret earlier in the same frame is discarded before it can be acted on: a spinning turret never settles on a target. Nothing draws a turret unless the type also sets [`Turret=yes`](/keys/turret/), so on a type without one the spin is invisible.

No stock type sets the flag.
