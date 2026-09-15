---
key: RoughTile
summary: Tile set whose first tile is a theater's plain rough ground.
see_also: [ClearToRoughLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Only the set's first tile is used. It is the unbroken rough ground that [`ClearToRoughLat`](/keys/cleartoroughlat/) blends against, and the tile a rough cell reverts to once all four of its orthogonal neighbors are rough as well.

The random map generator paints it in patches over clear ground, in every biome. On temperate and mutated maps it competes with [`GreenTile`](/keys/greentile/) and on desert with [`SandTile`](/keys/sandtile/), never with both at once. On tundra and taiga maps it is the only ground scatter besides [`Rocks`](/keys/rocks/).
