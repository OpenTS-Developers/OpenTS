---
key: ClearToSandLat
summary: Tile set holding the sixteen blend pieces laid where sand meets other terrain.
see_also: [ClearToRoughLat, SandTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Sand blends by the rule [`ClearToRoughLat`](/keys/cleartoroughlat/) describes, against the single plain [`SandTile`](/keys/sandtile/) ground tile. When the set is unresolved the whole sand family is skipped, and sand keeps its square edges rather than taking unrelated tiles.

The sixteen tiles are also the terrain test the random map generator uses to scatter loose rock. A cell holding one of them takes one of the five sand rock overlays, where plain clear ground and green blend cells take the five clear rock overlays instead.
