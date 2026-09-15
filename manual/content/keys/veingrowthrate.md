---
key: VeinGrowthRate
summary: Parsed rate that the engine never uses.
no_effect: true
see_also: ["system:veins", "VeinholeGrowthRate"]
when_omitted:
  kind: value
  value: "1"
---

The value is stored and never read. Vein growth is timed per monster from [`VeinholeGrowthRate`](/keys/veinholegrowthrate/) in `[General]`.
