---
key: Morphable
summary: Ground that the terrain-reshaping tools are allowed to move, and that will take a smudge.
see_also: [AllowTiberium, AllowBurrowing, Height, Width]
when_omitted:
  kind: value
  value: "no"
---

The value is read once per tile set and applied to every tile the set produces. Three things read it, through four tests in all:

- Raising or lowering the ground refuses to move a cell whose tile forbids it. Two predicates ask the question, the one the smoothing pass uses and the map generator's own. What else each refuses differs, but neither moves a cell the flag pins. Objects standing on a cell can veto a height change as well, and the tile pins it whether or not anything stands there.
- A crater or scorch mark is refused on such a cell. [`Height`](/keys/height/#scope-smudgetype) explains the rest of the placement test and how a smudge covering several cells is checked.
- A structure that paves the ground it is placed on (one that turns its cells into a tile of its own) may not be built where the tile forbids reshaping.

A cell whose tile index is unresolved is treated differently by each. Reshaping and building both let it through as though the flag were set; the smudge test instead returns the flag on the theater's very first tile.

```ini title="TEMPERAT.INI"
[TileSet0631]      ; example cliff set
SetName=Riverbank cliffs
FileName=RVCLIF
TilesInSet=8
Morphable=no       ; cliffs stay where the artwork puts them
```

Nothing else reads the flag: it does not affect movement, buildability in general, Tiberium growth, or which tiles the blend families may replace.
