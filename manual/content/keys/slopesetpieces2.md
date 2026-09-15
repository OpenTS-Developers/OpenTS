---
key: SlopeSetPieces2
summary: Tile set holding the marble madness counterparts of the slope pieces.
see_also: [SlopeSetPieces, MMRampBase]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it. The stamping substitutions then fire on the theater's fifth and eighth tiles. The theater's first nine tiles are read against the slope shadow table whenever they draw, so ordinary ground can pick up a slope shadow.
---

The second slope set is read in three places, each mirroring what [`SlopeSetPieces`](/keys/slopesetpieces/) receives. Stamping the sixth piece on subtile `0`, `3`, `6` or `9` substitutes the second piece of [`MMRampBase`](/keys/mmrampbase/) for it, and stamping the ninth piece on a subtile below `4` substitutes the first. Any tile in the first ten places from this role also draws its shadow from the same fixed per-piece table the ordinary slopes use, in place of the shadow a set marked [`ShadowCaster=yes`](/keys/shadowcaster/) would draw. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index, and covers the marble madness artwork this set draws on.

The collapsing-cliff replacement does not use this set; it always takes its pieces from `SlopeSetPieces`.
