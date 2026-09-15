---
key: BlackTile
summary: Tile-set number that resolves to a tile index nothing reads.
no_effect: true
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

The role resolves like every other in the [theater control file](/formats/theater-control/): when the numbered sections are read, the named set's first tile is recorded. No part of the engine reads the result afterwards.
