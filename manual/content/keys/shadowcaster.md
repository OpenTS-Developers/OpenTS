---
key: ShadowCaster
summary: Tile set whose tiles darken the cells below them with a cliff shadow.
see_also: [ShadowTiles, TilesInSet]
when_omitted:
  kind: value
  value: "no"
---

A set with the flag registers the index of its first tile in a table of shadow-casting sets. Its tiles are marked as casters, but only if [`ShadowTiles`](/keys/shadowtiles/) also gives a non-zero figure. Without that, the set takes up a table slot and no tile of it ever casts anything.

What a marked tile draws is not chosen from the artwork of the set. The engine keeps a fixed table of forty shadow descriptions, indexed by a tile's zero-based offset from the first tile of the caster set it matched: the set's first tile is position 0. Each entry names a frame of the shared cell-shadow shape, the sub-tile the shadow belongs on, and where to place it. Only positions 20 through 32 of the table hold a frame, so the twenty-first through thirty-third tiles of the set are the ones that draw shadows. Positions 20 and 21 share the first frame, and each later position draws the next in sequence. Those same positions hold the list of cells to mark as overshadowed, so those cells redraw along with the cliff. A tile at any other position is marked as a caster, is asked for a shadow, and draws nothing. The shadow is drawn only on the sub-tile the entry names. Sub-tile 0 is the tile's top-left cell and sub-tile 1 is the cell to its right, so a cell showing any other sub-tile of the same tile gets none. An author ordering a cliff set can read the whole layout off the table:

| Position | Shadow shape frame | Sub-tile |
| --- | --- | --- |
| 20 | 1st | 0 |
| 21 | 1st | 0 |
| 22 | 2nd | 1 |
| 23 | 3rd | 1 |
| 24 | 4th | 1 |
| 25 | 5th | 0 |
| 26 | 6th | 0 |
| 27 | 7th | 1 |
| 28 | 8th | 0 |
| 29 | 9th | 0 |
| 30 | 10th | 1 |
| 31 | 11th | 1 |
| 32 | 12th | 0 |

```ini title="TEMPERAT.INI"
[TileSet0010]        ; example cliff set
SetName=Cliffs
FileName=CLIFF
TilesInSet=40
ShadowCaster=yes
ShadowTiles=40
```

:::danger[A theater may declare only five shadow-casting sets]
Nothing counts the caster sets a theater declares. A sixth set with the flag writes a sixth entry past the end of that five-entry table, into whatever static storage follows it. Nothing ever reads that entry back either (the lookup scans five slots), so a sixth caster set casts no shadow of its own and damages the engine's static data on the way. The count is taken over the whole theater control file, including sets whose `ShadowTiles` figure leaves their tiles casting nothing.
:::

:::caution[Caster sets within forty tiles of each other are confused]
A tile looks itself up by walking the caster table from the first slot and taking the first set it lies within forty tiles of. Where two caster sets are loaded close together, tiles of the later one fall inside that range of the earlier one and are given the earlier set's shadow description instead of their own.
:::
