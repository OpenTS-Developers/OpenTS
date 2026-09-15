---
key: CliffSet
summary: Tile set holding the theater's forty cliff faces.
see_also: [CliffRamps, CrystalCliff, WaterCliffs, DestroyableCliffs]
when_omitted:
  kind: value
  value: "-1"
  note: The role stays unresolved, because no tile set number can match it, and the generator's tile-level cliff test then answers yes for the theater's first thirty-nine tiles.
---

The forty pieces are ordered by shape. Every cliff decision names its piece by a shape number counting from 1 at the first piece, and the piece placed for shape `n` sits `n - 1` places past the set's first tile. The random map generator's cliff pass turns the pattern of high ground around a cell into one of those shape numbers and stamps the piece that shape names. It also recognizes a cliff by testing whether a tile falls in the forty places from here, and reads the crystal-cliff shapes as a distance from this same role, one less than the shape number. [Theater control files](/formats/theater-control/) explains how a `[General]` role is resolved to a live tile index.

Every path that reads the role belongs to random map generation. A cliff placed on a hand-authored map still stops movement, because that comes from the land type the cliff artwork itself has rather than from this setting.

:::danger[An unresolved role is used as a tile index anyway]
The cliff pass indexes the tile heap directly with this role plus the shape it chose, without first testing that the role resolved. With the role unresolved every placement indexes the heap at the shape number minus two, and the first of the forty shapes indexes the heap at `-1`. Leaving this role out while [`CrystalCliff`](/keys/crystalcliff/) resolves is worse. The crystal substitution then derives its shape by subtracting an unresolved figure, and the fixed table of forty shape offsets is read past its end by however large the crystal role's own index is.
:::
