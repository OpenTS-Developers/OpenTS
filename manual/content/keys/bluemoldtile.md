---
key: BlueMoldTile
summary: Tile set whose first tile is a theater's plain blue mold ground.
see_also: [ClearToBlueMoldLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Only the set's first tile is used, as the unbroken mold that [`ClearToBlueMoldLat`](/keys/cleartobluemoldlat/) blends against and the tile a mold cell reverts to when mold surrounds it.

Blue mold is a mutated-biome feature of the random map generator, and a mutated map uses the temperate theater. A cell is molded when all four of its orthogonal neighbors are clear or already molded. The cell's own tile is never examined, so mold can overwrite whatever was there. The generator then makes up to five random draws among the molded cells, dressing each drawn cell with a mutated terrain object or a patch of large Tiberium.
