---
key: AV
summary: Narrows the target categories the weapon contributes to a scan down to vehicles alone.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

When a target scan arrives with no target category of its own, each of the object's weapon slots contributes the categories its projectile allows. A projectile marked anti-vehicle contributes vehicles and returns immediately, so [`AA`](/keys/aa/) and [`AG`](/keys/ag/) on that same projectile contribute nothing at all.

That is the whole of the setting. It places no restriction on what the weapon may fire at: a target reached by an order, a team script or retaliation is still governed by `AA` and `AG`.
