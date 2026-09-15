---
key: MissileROTVar
summary: How far a homing projectile's rate of turn swings above its nominal value.
see_also: [MissileSpeedVar, ROT]
when_omitted:
  kind: value
  value: ".25"
---

A projectile with a [`ROT`](/keys/rot/#scope-bullettype) above zero is steered by the homing flight model, and this figure is what makes it weave rather than track straight. The rate of turn is scaled by a factor that swings over a fifteen-frame cycle: `1` at the bottom of the cycle, and `1` plus twice this figure at the top. The swing therefore stays at or above the nominal rate for any positive value. At `0` the factor is a constant `1`, so the projectile still turns at its nominal rate but never weaves.

Each projectile enters the cycle at its own point, so missiles launched together do not weave in step.

Two adjustments sit outside the cycle. A projectile still working up to speed after launch is given no turn at all. One within a cell of its target turns half again as fast as the cycle asks.
