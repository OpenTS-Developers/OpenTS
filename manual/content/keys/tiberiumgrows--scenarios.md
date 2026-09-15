---
key: TiberiumGrows
scope: scenarios
label: Fast growth
see_also: ["system:tiberium", "Growth", "TiberiumGrowthEnabled"]
when_omitted:
  kind: value
  value: "no"
  note: The special options are initialized with this built-in default when the game starts.
---

Each type's [`Growth`](/keys/growth/) delay is multiplied by `0.3` when it is reloaded, which is the whole of the switch's effect. It does not decide whether Tiberium grows: growth runs at the full delay with the switch off and stops only when [`TiberiumGrowthEnabled=no`](/keys/tiberiumgrowthenabled/).

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission. A network lobby switches fast growth on for the game it launches; a skirmish gets no such hand-over and runs at the full delay, unless a lobby match earlier in the same session left the switch on. The map's entry is ignored in both.
:::
