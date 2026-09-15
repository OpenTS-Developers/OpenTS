---
key: TilesInSet
summary: Count of tiles a tile set contributes to the theater, and the marker that ends the theater read.
see_also: [LastTilesInSet, FileName, SetName]
when_omitted:
  kind: value
  value: "-1"
  note: The theater read stops at this section, and no tile set numbered at or above it is loaded.
---

The figure is how many tiles the set defines. Each one becomes a tile type in the theater heap, numbered consecutively from wherever the previous set left off. Each one is looked for on disk as [`FileName`](/keys/filename/) plus a two-digit index counting from `01`. A set is registered before its count is read, so a set whose count says zero still claims a position in the theater's tile numbering, but contributes no tiles. This is the numbering a [`General`](/formats/theater-control/) role resolves against: `ShorePieces=NNNN` names a tile set, and the engine records the number of that set's first tile.

```ini title="TEMPERAT.INI"
[TileSet0631]      ; example set
SetName=Riverbank cliffs
FileName=RVCLIF
TilesInSet=8       ; RVCLIF01 through RVCLIF08
```

The count is trusted as written. A count larger than the artwork on disk still creates that many tile types; the surplus ones hold no image, report zero width and height, and draw nothing where a cell places them. A negative count other than the omission marker ends the read at that section: the set contributes nothing, and no set numbered at or above it is loaded.
