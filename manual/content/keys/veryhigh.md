---
key: VeryHigh
summary: Raises a homing projectile's cruising clearance and delays the dive onto its target.
see_also: [ROT, High, Airburst]
when_omitted:
  kind: value
  value: "no"
---

Only a projectile steered by the homing flight model is affected: one whose [`ROT`](/keys/rot/#scope-bullettype) is above zero.

An ordinary homing projectile follows the terrain while it is still more than three cells from its target horizontally. It holds as many terrain levels of clearance above the ground ahead of it as there are whole cells left to run, up to five (a little over two cells of height). Inside three cells it stops following the ground and pitches straight at the target.

The setting changes both figures. The clearance becomes a flat ten terrain levels, about four cells of height, however far out the projectile is, and the dive is not begun until the projectile is within six cells. A shot fired across a ridge therefore rides over it rather than into it.

It also exempts the projectile from the stall check, which forces a homing projectile that has stopped gaining on its target to detonate.

A projectile chasing an aircraft skips terrain-following altogether and flies straight at its quarry, so on that shot only the exemption from the stall check remains.

The setting is unrelated to [`High`](/keys/high/#scope-bullettype), which decides whether a projectile passes over walls and other tall overlays.
