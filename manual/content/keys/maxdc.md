---
key: MaxDC
summary: The frames between one particle's damage applications.
see_also: ["Damage", "Warhead", "FinalDamageState", "MaxEC"]
when_omitted:
  kind: value
  value: "0"
---

A particle holds a countdown seeded from this figure, steps it down once a frame, and applies [`Damage`](/keys/damage/#scope-particletype) through its [`Warhead`](/keys/warhead/#scope-particletype) when it reaches zero, reloading the countdown as it does. `MaxDC=1` therefore scorches whatever shares the cell every frame, and `60` once every four seconds. A [`Gas`](/keys/behaveslike/#scope-particletype) particle applies the damage to every object in its cell; a `Fire` particle spares only the object its own system is attached to, and stops once its states pass [`FinalDamageState`](/keys/finaldamagestate/).

`WeakGas` particles run the gas logic with the damage step switched off and never apply anything, whatever this says. `Web` particles apply their warhead every frame and do not use the countdown. `Smoke`, `Spark` and `Railgun` particles have no damage path at all.

:::caution[Leaving it out disables the damage]
The countdown holds unsigned values only. Seeded from zero it does not fire on the first step but wraps to 65,535, and no particle lives long enough to bring it back to zero. A type given damage and a warhead but no interval therefore never harms anything.
:::
