---
key: Rocks
summary: Tile set whose first tile is the rock ground the random map generator paints on snow maps.
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Only the set's first tile is used, and only by the random map generator, on tundra and taiga maps. It spreads in patches the same way ground scatters do elsewhere, and it spends the seed's per-cell sand chance to do it. That is the same chance that places [`SandTile`](/keys/sandtile/) on the maps that have sand, so a snow theater needs this role where a desert one needs that one.

Unlike the ground types it sits beside, rock has no blend family: a rock patch meets its neighbors along the cell edge.
