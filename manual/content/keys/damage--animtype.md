---
key: Damage
scope: animtype
label: Animation damage
when_omitted:
  kind: value
  value: "0.0"
---

The damage accumulates frame by frame while the animation plays, and is dealt in whole points at the animation's position with the [`FlameDamage2`](/keys/flamedamage2/) warhead, or [`C4Warhead`](/keys/c4warhead/) for the animation named `INVISO`. The accumulation runs five times as fast when the animation sits on a terrain object. A bouncing animation instead deals the figure with its own [`Warhead`](/keys/warhead/) to each object within [`DamageRadius`](/keys/damageradius/) of an impact.
