---
key: MMRampBase
summary: Tile set holding the marble madness counterparts of the plain ramps.
see_also: [RampBase, SlopeSetPieces2, RampSmooth]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and the two indices it derives are `-1` and the theater's first tile.
---

One path reads this role, and it is the marble madness mirror of the substitution that [`RampBase`](/keys/rampbase/) receives. Stamping the sixth piece of [`SlopeSetPieces2`](/keys/slopesetpieces2/) onto a cell whose subtile is `0`, `3`, `6` or `9` replaces it with the second piece from this role. Stamping the ninth piece onto a subtile below `4` replaces it with the first. Nothing else reads it. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index, and covers the marble madness artwork the second set draws on.

:::danger[The derived tile index is never checked]
The substitution writes the index into the cell without testing that the role resolved. In a theater that resolves `SlopeSetPieces2` but leaves this one unresolved, stamping the ninth piece on a low subtile writes `-1` into the cell, and the cell then hands that index to the tile heap. Stamping the sixth piece silently turns the cell into the theater's first tile instead.
:::
