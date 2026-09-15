---
key: RampSmooth
summary: Tile set holding the twelve pieces that blend a straight ramp into flat ground.
see_also: [RampBase, MMRampBase, SlopeSetPieces]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and every piece but the first derives a tile index landing in the theater's first eleven tiles.
---

Twelve pieces run from this role, three for each of the four straight ramp directions and in that order: west, north, east, then south. Within a direction the piece is picked by which of the two cells along the ramp's axis has no ramp of its own: the cell the ramp descends toward, the cell behind it, or both. The three pieces follow that order, so the first answers a flat downhill cell, the second a flat cell behind, and the third both. When both neighbors are ramps the cell goes back to its plain piece from [`RampBase`](/keys/rampbase/) instead. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

Only the four straight ramps are blended this way. Corner, steep and double ramps keep their plain artwork whatever surrounds them.

:::danger[The derived tile index is never checked]
The blending pass computes the piece as an offset from this role and writes it into the cell without testing that the role resolved, then reaches into the tile heap for the artwork of whatever index came out. In a theater that resolves [`RampBase`](/keys/rampbase/) but leaves this one unresolved, the index falls somewhere in the theater's first eleven tiles, and a west-descending ramp whose downhill neighbor alone is flat writes `-1`.
:::
