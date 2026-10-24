---
key: VeteranCap
summary: The highest experience total any promotion may leave an object at.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

Every experience gain is limited to this value: kills, veterancy-crate steps, armory promotions, [`InitialVeteran`](/keys/initialveteran/), a reinforcement's [`VeteranLevel`](/keys/veteranlevel/), the Drop Pods superweapon, the Make Elite trigger action, and experience from a map's placed-object record. Veteran rank starts at `1` experience and elite at `2`, so the value sets the highest rank any promotion can reach:

| `VeteranCap` | Highest rank |
| --- | --- |
| Below `0` | Below rookie. Every experience gain leaves the object below rookie. |
| `0` up to but not including `1` | Rookie. Promotions never raise an object above rookie. |
| `1` up to but not including `2` | Veteran |
| `2` and above | Elite |

A saved game restores the stored total without applying the limit. An object above the limit drops to it on its next credited kill.

:::caution[Set VeteranCap to 2 for elite rank]
With `VeteranCap` unset, no promotion raises an object past veteran: kills, crates, armories, reinforcements, and triggers all stop at `1`.
:::

:::caution[A ceiling below an object's rank demotes it]
The limit applies to the running total, not to the experience a kill adds. An object above the limit drops to it on its next credited kill, or at once when a source sets its rank; Make Elite does this to an elite when `VeteranCap` is below `2`. Any drop to veteran takes the elite weapon and [`EliteAbilities`](/keys/eliteabilities/) with it.
:::
