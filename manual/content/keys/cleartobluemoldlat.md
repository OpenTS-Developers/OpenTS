---
key: ClearToBlueMoldLat
summary: Tile set holding the sixteen blend pieces laid where blue mold meets other terrain.
see_also: [ClearToRoughLat, BlueMoldTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Blue mold blends by the rule [`ClearToRoughLat`](/keys/cleartoroughlat/) describes, against the single plain [`BlueMoldTile`](/keys/bluemoldtile/) ground tile. It is the last family evaluated in the pass, so it sees whatever tile the six before it settled on. It is also one of the four families that keep blending when their set is unresolved.
