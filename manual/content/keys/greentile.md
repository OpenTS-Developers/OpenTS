---
key: GreenTile
summary: Tile set whose first tile is a theater's plain green ground.
see_also: [ClearToGreenLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Only the set's first tile is used, as the unbroken green ground that [`ClearToGreenLat`](/keys/cleartogreenlat/) blends against and the tile a green cell reverts to when it is surrounded by green.

The random map generator paints it in patches on temperate and mutated maps; the desert weight is zero. It tests its chance first, before rough and sand, so a cell that draws green never gets the chance to become either of those. Tundra and taiga maps never place it.
