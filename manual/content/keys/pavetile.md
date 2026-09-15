---
key: PaveTile
summary: Tile set whose first tile is a theater's plain pavement.
see_also: [ClearToPaveLat, MiscPaveTile]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The set's first tile is the unbroken pavement that [`ClearToPaveLat`](/keys/cleartopavelat/) blends against and the tile a paved cell reverts to when pavement surrounds it. It is also the tile the random map generator lays over the footprint of a generated town. Cells that end up with an unpaved neighbor are dropped from the list the generator works through; the ground under them stays paved. It then drops [`MiscPaveTile`](/keys/miscpavetile/) weathering over what is left.

Sixteen tiles are counted from here for the terrain test that asks whether a cell is pavement, which is what the generator's road and building placement passes check before committing to a cell. The town and junction passes compare against the first tile directly instead.

:::caution[Clear ground reads as pavement when the set is unresolved]
That terrain test is not gated on the set resolving. With the role unresolved it accepts everything from the start of the tile heap up to index 14, so the theater's first fifteen tiles — clear ground among them — answer that they are pavement.
:::
