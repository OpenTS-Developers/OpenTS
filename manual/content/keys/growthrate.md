---
key: GrowthRate
summary: Parsed rate that the engine never uses.
no_effect: true
see_also: ["system:tiberium", Growth]
when_omitted:
  kind: value
  value: "2"
---

The name marks it as the interval between Tiberium growth passes, expressed in minutes. Nothing reads the stored figure.

Growth timing is per Tiberium type and counted in frames instead. A type's timer reloads with its own [`Growth`](/keys/growth/) delay after every pass, shortened to 30% of that delay while the scenario's [`TiberiumGrows`](/keys/tiberiumgrows/) setting is on. The scenario's [`TiberiumGrowthEnabled`](/keys/tiberiumgrowthenabled/) switch decides whether any pass runs, and [Growth](/systems/tiberium/#growth) explains how a pass picks the cells it ripens.
