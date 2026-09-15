---
key: SandTile
summary: Tile set whose first tile is a theater's plain sand ground.
see_also: [ClearToSandLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Only the set's first tile is used, as the unbroken sand that [`ClearToSandLat`](/keys/cleartosandlat/) blends against and the tile a sand cell reverts to when it is surrounded by sand.

The random map generator paints it in patches on desert maps. The temperate and mutated weights are zero, so it never fires there. On tundra and taiga maps the same per-cell sand chance is spent on [`Rocks`](/keys/rocks/) instead, so a snow theater never needs this set for generated terrain.
