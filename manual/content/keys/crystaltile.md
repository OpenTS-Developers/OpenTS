---
key: CrystalTile
summary: Tile set whose first tile is a theater's plain crystal ground.
see_also: [ClearToCrystalLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Only the set's first tile is used, as the unbroken crystal that [`ClearToCrystalLat`](/keys/cleartocrystallat/) blends against and the tile a crystal cell reverts to when crystal surrounds it.

Crystal is a mutated-biome feature of the random map generator, which maps to the temperate theater. The generator floods a spread of cells with this tile, then makes up to five random draws among those cells. Each drawn cell is dressed with a crystal terrain object when it is still plain crystal and not reserved for something else.
