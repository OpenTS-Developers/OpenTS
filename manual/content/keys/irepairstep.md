---
key: IRepairStep
summary: The strength one repair step restores to an infantry.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "1"
---

Infantry types replace [`RepairStep`](/keys/repairstep/) with this figure wherever a step is applied to them. In practice that means [a hospital visit](/systems/repair/#hospitals-and-armories) and standing on Tiberium with [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype). Both sites raise anything below `1` to `1`, so a value of `0` still heals a point at a time.

Infantry also replace the credit price of a step with zero, so this is the whole of what a hospital charges and gives.
