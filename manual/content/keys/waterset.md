---
key: WaterSet
summary: Fourteen-tile set of open water.
see_also: [ShorePieces, ClearTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

A cell holding any of the fourteen is open water. That answer is what the random map generator reads from each of a cell's eight neighbors when it works out which shoreline piece belongs there. Together with shore and the four waterfall sets it also answers the broader question of whether a cell holds any water at all. A transport reads that question before taking on a passenger: one standing on water, on a [`ShorePieces`](/keys/shorepieces/) tile, or on a waterfall refuses.

The set's first tile is plain open water: it is what `[Map] Fill=Water` stamps into every cell of a map, and what the generator writes when it grows a body of water outward. [`ClearTile`](/keys/cleartile/) covers what fills a map given any other value, and when during the load that fill happens.

:::caution[Clear ground reads as water when the set is unresolved]
Neither water test is gated on the role resolving. With it unresolved both accept everything from the start of the tile heap up to index 12. The theater's first thirteen tiles, clear ground among them, answer that they are water, and transports standing on plain ground refuse their passengers.
:::
