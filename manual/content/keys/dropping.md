---
key: Dropping
summary: Marks the projectile as a bomb that falls from a height rather than flying to its target.
see_also: [ROT, Arm]
when_omitted:
  kind: value
  value: "no"
---

The setting changes the launch and the fuse. The projectile is released along the firer's own heading rather than toward the target, exactly as a homing projectile is. The firing sound is played from the firer's center instead of from the barrel. It then ignores its fuse completely: the proximity trip that ends an ordinary flight never ends this one. Nothing about [`Arm`](/keys/arm/) or the distance to the target decides when the shot goes off.

:::caution[A dropping projectile detonates on its first logic frame]
Detonation is forced as soon as the projectile is not falling, and no path in the engine ever sets a projectile falling. The shot therefore goes off one frame's travel from where it was released, whatever else the section says.
:::

The stock game does not use the flag. The Orca Bomber drops its bombs with an ordinary projectile (`Cannon2`, with no `Dropping`, `Arcing` or `ROT`). The aircraft throws each bomb forward at its own flying speed, and the ordinary flight model pulls it down by gravity frame by frame. That fall is the drift down from the transport. No stock projectile sets `Dropping=yes`, and the engine routine that would set a projectile falling, hanging the parachute animation on it, is reachable only for aircraft passengers, which a projectile never is.
