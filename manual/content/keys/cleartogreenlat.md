---
key: ClearToGreenLat
summary: Tile set holding the sixteen blend pieces laid where green terrain meets other terrain.
see_also: [ClearToRoughLat, GreenTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Green terrain blends by the rule [`ClearToRoughLat`](/keys/cleartoroughlat/) describes, against the single plain [`GreenTile`](/keys/greentile/) ground tile. When the set is unresolved the whole green family is skipped, and green terrain keeps its square edges.

A cell holding one of these sixteen tiles is treated as clear ground when the random map generator scatters loose rock. It takes a clear rock overlay rather than the sand rock overlays a [`ClearToSandLat`](/keys/cleartosandlat/) cell would take.
