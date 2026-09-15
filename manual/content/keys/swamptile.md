---
key: SwampTile
summary: Nine-tile set of swamp ground, the first tile plain and the rest decorative.
see_also: [WaterToSwampLat]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The first tile is plain swamp: what [`WaterToSwampLat`](/keys/watertoswamplat/) blends against, and what a swamp cell reverts to when swamp surrounds it. The eight that follow are decorative. The random map generator writes the plain tile over every flooded cell of a swamp spread, then places the decorative tiles only where every cell under the candidate patch is still plain swamp.

Whether the role resolved at all gates the terrain test that asks whether a cell is swamp, the one test in this group that is gated. A theater with no swamp always answers no, which is why a theater may leave both swamp roles out without the test misreading its ordinary ground.
