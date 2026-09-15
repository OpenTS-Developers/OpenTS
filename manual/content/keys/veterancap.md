---
key: VeteranCap
summary: The highest experience total a credited kill may leave an object at.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

Experience earned from kills is clamped to this ceiling immediately after it is added. Because veteran begins at `1` and elite at `2`, the value doubles as the highest rank combat can reach: `1` stops at veteran, and `2` allows elite. The ranks written by crates, armories, reinforcements, and drop pods are assigned directly and are not clamped.

:::caution[The default stops promotion at veteran]
With no `VeteranCap` in the rules, no object can ever be promoted to elite by fighting. Set it to `2` for combat promotion to reach elite.
:::

:::danger[A ceiling below an object's rank demotes it]
The clamp is applied to the running total rather than to the increment, so an object already above the ceiling is pulled down to it by its next credited kill. Under the default ceiling, an elite [`Trainable=yes`](/keys/trainable/) object that scores a kill drops to veteran and loses its elite weapon and [`EliteAbilities`](/keys/eliteabilities/).
:::
