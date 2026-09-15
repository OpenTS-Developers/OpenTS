---
key: WaterToSwampLat
summary: Tile set holding the sixteen blend pieces laid where swamp meets other terrain.
see_also: [ClearToRoughLat, SwampTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Swamp blends by the rule [`ClearToRoughLat`](/keys/cleartoroughlat/) describes, with a wider neighbor test than the other families. A neighbor counts as swamp when it falls anywhere in the run of ten tiles beginning at the plain [`SwampTile`](/keys/swamptile/) tile. That is one tile more than the `SwampTile` set holds, so the tenth entry is whatever tile occupies the position after those nine. The eight decorative variants sit inside that run: they count as swamp to the test, and no blend edge forms against them.

These sixteen tiles also answer as swamp to the terrain test that asks whether a cell is swamp, which is what keeps a blended shoreline of swamp behaving like the swamp it borders rather than like the water.

Swamp is one of the four families that keep blending even when their set is unresolved, and here the terrain test misreads with it. That test is gated on the swamp ground role alone, so a theater resolving `SwampTile` but not this set reports its own first fifteen tiles as swamp.
