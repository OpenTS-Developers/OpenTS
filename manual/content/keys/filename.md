---
key: FileName
summary: Stem of the artwork file names a tile set loads its tiles from.
see_also: [TilesInSet, SetName, NonMarbleMadness]
when_omitted:
  kind: value
  value: TILE
---

Each tile of the set is looked for as this stem plus a two-digit index counting from `01`, extended with the theater's own suffix. [Theater control files](/formats/theater-control/) covers the lettered alternates built on the same stem and the fallback extension tried when the theater file is missing.

```ini title="TEMPERAT.INI"
[TileSet0631]      ; example set
SetName=Riverbank cliffs
FileName=RVCLIF    ; loads RVCLIF01.TEM through RVCLIF08.TEM
TilesInSet=8
```

Nothing checks that the stem produced a file. A tile whose artwork is absent still becomes a tile type, reports zero width and height, and draws nothing where a cell places it.

The stem is capped at 63 characters; a longer value is cut short when it is read. A tile type keeps the whole path it loaded from, so that it can read its artwork again after the theater trims it out of memory.
