---
key: IceShoreSet
summary: Forty-eight-tile set of the land-side pieces laid where ice meets ground.
see_also: [Ice1Set, Ice3Set]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

One pass reads the role, and it runs only where the theater has ice growth enabled (`IsIceGrowthEnabled`). It considers a cell eligible when the cell is empty, is clear ground, or already holds one of these forty-eight tiles. The pass then builds a pattern from all eight neighbors, according to which of them fall inside the ice run described by [`Ice3Set`](/keys/ice3set/). That pattern selects the piece.

The lookup it uses is shared with the ice-side edge pieces of [`Ice1Set`](/keys/ice1set/) and its two companions, and it yields three kinds of result. A value from 1 to 46 is a piece, taken at that offset into this set, so offsets 0, 15 and 47 are never produced. A second result means no shore piece suits the pattern and the cell should become plain ice instead, which turns the cell over to the ice pass and re-runs the shore decision around it. The third means nothing suits and the cell is left as it is.

:::caution[The pass is not gated on the role resolving]
Only `IsIceGrowthEnabled` is checked. With the role unresolved the set's forty-eight-tile span is counted from `-1`, so the eligibility test sweeps tile indices `0` through `46`: the theater's first forty-seven tiles. The pass writes the lookup value on top of the role, so each piece lands one tile below the offset the lookup names, and unrelated ground tiles are picked up as ice shore and overwritten with other unrelated tiles.
:::
