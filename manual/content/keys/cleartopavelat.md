---
key: ClearToPaveLat
summary: Tile set holding the sixteen blend pieces laid where pavement meets other terrain.
see_also: [ClearToRoughLat, PaveTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

Pavement blends by the rule [`ClearToRoughLat`](/keys/cleartoroughlat/) describes, and its neighbor test is the widest of the families. Besides the plain [`PaveTile`](/keys/pavetile/) tile and these sixteen, a neighbor counts as pavement when it is any of:

- the fourteen [`MiscPaveTile`](/keys/miscpavetile/) pieces,
- the fourteen [`Medians`](/keys/medians/) pieces,
- the first eight paved road tiles.

Laying a road or a median strip through a paved square therefore does not cut a blended edge into it.

When the set is unresolved the whole pavement family is skipped and pavement keeps its square edges.
