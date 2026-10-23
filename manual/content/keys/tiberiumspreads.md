---
key: TiberiumSpreads
summary: Allows a ripe Tiberium cell to seed its neighbors.
see_also: ["system:tiberium", "TiberiumGrowthEnabled", "SpreadPercentage"]
when_omitted:
  kind: value
  value: "yes"
  note: Each scenario starts with this built-in default.
---

With `TiberiumSpreads=no`, Tiberium cells still grow but never [spread](/systems/tiberium/#spread) onto neighboring cells. Blossom trees are not affected and still seed the ground around them; [other sources of Tiberium](/systems/tiberium/#other-sources-of-tiberium) covers them.

:::caution[The entry is read in campaigns only]
Only a single-player mission reads `[SpecialFlags]` from the map. Skirmish and multiplayer games always have this switch on.
:::
