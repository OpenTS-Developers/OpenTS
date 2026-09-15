---
key: ClearToCrystalLat
summary: Tile set holding the sixteen blend pieces laid where crystal terrain meets other terrain.
see_also: [ClearToRoughLat, CrystalTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Crystal terrain blends by the rule [`ClearToRoughLat`](/keys/cleartoroughlat/) describes, against the single plain [`CrystalTile`](/keys/crystaltile/) ground tile. One addition no other family has: four tiles of the crystal cliff set count as crystal on the half of the cliff that has crystal. The cliff's first tile counts only for subtiles below 2 and its second only for subtiles from 2 up. A subtile is one cell of a multi-cell tile piece, numbered across each row from the top left. Its fifth counts only on odd subtiles and its sixth only on even ones. A crystal field running up to a crystal cliff therefore blends into the rock face instead of stopping short of it.

Crystal is one of the four families that keep blending even when their set is unresolved, so a theater with crystal ground and no crystal LAT set misplaces tiles rather than leaving the ground alone.
