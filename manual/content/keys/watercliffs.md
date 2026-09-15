---
key: WaterCliffs
summary: Tile set holding the cliff faces that drop straight into water.
see_also: [CliffSet, CliffRamps, DestroyableCliffs]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and no tile is recognized as a water cliff.
---

Twenty-eight pieces run from this role, and they answer the same cliff test as [`CliffSet`](/keys/cliffset/) does. Random map generation is the only thing that asks. A cell with a water cliff never counts as the high ground that the terrain pass raises its neighbors to join, and a shore piece is refused on such a cell. The generator therefore leaves rock rather than beach where the land drops into the water. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

Movement across the finished map reads the land type in the tile artwork, so an unresolved role changes what the generator builds and nothing else.
