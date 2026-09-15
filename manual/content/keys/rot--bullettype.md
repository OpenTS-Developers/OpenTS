---
key: ROT
scope: bullettype
label: Projectile rate of turn
when_omitted:
  kind: value
  value: "0"
---

Any figure above zero makes the projectile a homing one, and that is the switch between the engine's two flight models. A homing projectile is flown by the missile autopilot, which steers it toward its target and lifts it over terrain in the way. Everything else follows the arc it was launched along until it hits something.

The figure is not applied flat. [`MissileROTVar`](/keys/missilerotvar/) swings the rate of turn upward from the written figure over a fifteen-frame cycle, running between the written figure and one plus twice that share of it, never below it. The projectile's own identifier is mixed into the cycle's phase, so missiles fired together do not all weave in unison. Over the last cell of the approach the turn rate is raised by half, so that a target dodging at the last moment can still be caught.

Homing changes the launch as well. The shot leaves along the firer's turret facing rather than pointing at the target, and it takes the weapon's [`Speed=`](/keys/speed/#scope-weapontype) as written instead of having that speed recomputed from the weapon's range. Two figures are special-cased for aircraft. At `ROT=0` the shot takes the aircraft's own apparent speed and turret yaw. At `ROT=1` the aircraft aims the shot straight at its target and gives it the primary weapon's speed. Only a projectile at `ROT=1` or below is considered suited to a strafing run.
