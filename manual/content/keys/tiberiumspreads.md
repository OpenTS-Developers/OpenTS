---
key: TiberiumSpreads
summary: Allows a ripe Tiberium cell to seed its neighbors.
see_also: ["system:tiberium", "TiberiumGrowthEnabled", "SpreadPercentage"]
when_omitted:
  kind: value
  value: "yes"
  note: The special options are initialized with this built-in default when the game starts.
---

The switch is the first test a cell makes before it may [spread](/systems/tiberium/#spread); with it off, cells still ripen but never seed bare ground.

:::caution[The entry is read in campaigns only]
The `[SpecialFlags]` block is read from the map only in a single-player mission. Every multiplayer and skirmish game forces this switch on when the scenario starts.
:::
