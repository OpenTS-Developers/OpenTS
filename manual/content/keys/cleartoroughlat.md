---
key: ClearToRoughLat
summary: Tile set holding the sixteen blend pieces laid where rough ground meets other terrain.
see_also: [RoughTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

A LAT set is the strip of blend art that keeps one ground type from meeting another along a hard square edge. The [theater control file](/formats/theater-control/) names it by tile-set number, counted from 0 for the file's first `TileSet` section, and that number resolves to the index of the set's first tile.

Blending is a per-cell decision taken whenever a cell's attributes are recalculated. A cell takes part when it has either its family's plain ground tile or one of the sixteen tiles of the family's LAT set. Its four orthogonal neighbors are then examined in the order north, east, south, west. Every neighbor belonging to neither of those two groups contributes a bit: 1 for north, 2 for east, 4 for south, 8 for west. The total selects the tile at that offset into the LAT set, so a rough cell whose east and south neighbors are something other than rough takes the piece at offset 6. A cell whose four neighbors all match reverts to the plain ground tile. That case is handled by the plain tile rather than by offset 0, so the first tile of a LAT set is never chosen here and the blend art occupies offsets 1 through 15.

The families are evaluated one after another in a single pass: rough, sand, green, pavement, crystal, swamp, blue mold. Each one re-reads the tile the previous family left behind.

For this family the two groups are the single plain [`RoughTile`](/keys/roughtile/) ground tile and the sixteen tiles here. Nothing else counts as rough, so rough ground blends against pavement, water, and cliffs alike.

:::caution[Four of the seven families still run when their set is unresolved]
Sand, green, and pavement are skipped outright when their own LAT set is unresolved. Rough, crystal, swamp, and blue mold are not: a cell with the family's plain ground tile still computes an offset. The tile written is the one at heap index `offset - 1`, because the unresolved role is `-1` itself. That is one of the first fifteen tiles the theater loaded instead of blend art.
:::
