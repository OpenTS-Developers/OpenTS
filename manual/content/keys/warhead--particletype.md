---
key: Warhead
scope: particletype
label: Particle warhead
see_also: ["Damage", "MaxDC", "FinalDamageState", "BehavesLike"]
when_omitted:
  kind: value
  value: none
---

Only three of the seven [behaviors](/keys/behaveslike/#scope-particletype) reach a warhead at all. A `Gas` particle and a `Fire` particle set [`Damage`](/keys/damage/#scope-particletype) through it on the [`MaxDC`](/keys/maxdc/) cycle, so the warhead's armor multipliers and its [`Spread`](/keys/spread/#scope-warheadtype) decide what each target in the cell actually suffers. A `Web` particle applies it every frame at zero damage; without one it applies nothing at all. `WeakGas`, `Smoke`, `Spark` and `Railgun` particles never reach a damage path, so the assignment does nothing on those types.

A `Gas` or `Fire` particle with a warhead but no damage applies nothing, because both of those paths test the damage figure before they run. A `Web` particle is the reverse: its damage figure is never read, and the warhead alone decides what the repeated hit does.

`Warhead=none` and `Warhead=<none>` both clear the setting. Any other unrecognized name silently registers a WarheadType of that name with nothing configured in it, rather than being rejected.
