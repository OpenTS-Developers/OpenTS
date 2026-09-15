---
key: PavedRoadEnds
summary: The tile set that supplies the four caps that close the end of a paved road.
see_also: [PavedRoads, PavedRoadSlopes]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

Only the [random map generator](/formats/map-seed/) reads this role. It lays a cap counted from it wherever a road it has built stops, choosing the piece by the facing the road runs out in. Its two placement tests treat a cap already on the ground differently from road. A cap may be built over only when the caller allows it, and it needs every cell of its footprint at one height, so it will not straddle a slope. Unlike a road piece, a cap is not pavement for the purposes of the pavement blend.
