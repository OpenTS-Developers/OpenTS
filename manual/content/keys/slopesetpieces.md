---
key: SlopeSetPieces
summary: Tile set holding the ten slope pieces that carry ground between height levels.
see_also: [SlopeSetPieces2, RampBase, DestroyableCliffs]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and the four indices a collapsing cliff derives are `-1` and the theater's first three tiles.
---

Ten pieces run from this role, each a multi-cell slope rather than the single-cell ramps of [`RampBase`](/keys/rampbase/), and the engine names each one by its distance from the first. Four things read them. A collapsing cliff replaces itself with a pair of these pieces, which is what opens a way up the rock: the first two for one of the [`DestroyableCliffs`](/keys/destroyablecliffs/) faces, the fourth and third for the other. Stamping the sixth piece on subtile `0`, `3`, `6` or `9` substitutes the second plain ramp for it, and stamping the ninth piece on a subtile below `4` substitutes the first. Any tile in the first ten places from this role also draws its shadow from a fixed per-piece table, in place of the shadow a set marked [`ShadowCaster=yes`](/keys/shadowcaster/) would draw. Random map generation lays all ten while it builds terrain. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

:::danger[Collapsing a cliff reaches the tile heap unchecked]
The collapse takes its two replacement pieces straight out of the tile heap at this role and the three places after it, with no test that the role resolved. In a theater that resolves `DestroyableCliffs` but leaves this one unresolved, the first of the two faces reads the heap at `-1` when it comes down. The second lays the theater's second and third tiles across the wreckage as slopes.
:::
