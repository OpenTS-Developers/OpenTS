---
key: PavedRoads
summary: The tile set the fifteen paved road tiles are counted from.
see_also: [PavedRoadEnds, PavedRoadSlopes, ClearToPaveLat, PaveTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The role has one consequence away from the map generator: a paved road counts as pavement when the blend between pavement and clear ground is worked out. A cell showing [`PaveTile`](/keys/pavetile/) or one of the [`ClearToPaveLat`](/keys/cleartopavelat/) pieces looks at its four orthogonal neighbors and edges away from each one that is not pavement. The first eight pieces of this set count as pavement for that purpose, as do the [`MiscPaveTile`](/keys/miscpavetile/) and [`Medians`](/keys/medians/) pieces. Laying pavement against a road therefore leaves a clean join rather than a shoreline of transition pieces. The test is guarded, so a role that no tile set answered simply stops roads counting as pavement.

The [random map generator](/formats/map-seed/) reads the set more heavily. It lays its urban roads out of numbered pieces counted from this role. Its placement test admits road and [end caps](/keys/pavedroadends/) already on the ground only when the caller allows them to be built over, and refuses anything else that is not clear ground or pavement.
