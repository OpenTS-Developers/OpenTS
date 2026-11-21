---
key: TiberiumGrows
scope: scenarios
label: Fast growth
see_also: ["system:tiberium", "Growth", "TiberiumGrowthEnabled"]
when_omitted:
  kind: value
  value: "no"
  note: Each scenario starts with this built-in default.
---

`TiberiumGrows=yes` makes Tiberium grow faster: after each growth pass, a type waits 30% of its [`Growth`](/keys/growth/) delay instead of the full delay. The [`Spread`](/keys/spread/#scope-tiberium) delay between spread passes is unchanged.

The switch does not decide whether Tiberium grows at all. With it off, growth runs at the full `Growth` delay. Among the map's switches, only [`TiberiumGrowthEnabled=no`](/keys/tiberiumgrowthenabled/) stops growth. A Tiberium type with [`GrowthPercentage=0`](/keys/growthpercentage/) never grows.

:::caution[The entry is read in campaigns only]
Only a single-player mission reads `[SpecialFlags]` from the map. A game against other machines always uses fast growth. A skirmish uses the full delay.
:::
