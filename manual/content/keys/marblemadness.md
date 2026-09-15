---
key: MarbleMadness
summary: Parsed tile-set number that the engine resolves and never reads.
no_effect: true
see_also: [NonMarbleMadness, TilesInSet]
when_omitted:
  kind: value
  value: "65535"
  note: The marker for no substitute, which is what skips the resolution pass entirely.
---

The name promises the tile set that stands in for this one while the alternate terrain artwork is displayed. OpenTS has no such display. The value is a tile-set number (the `NNNN` of a `[TileSetNNNN]` section), copied onto the tiles of the set. Once the whole file has been read, each copy is resolved into the index of the matching tile in the named set: that set's first tile plus this tile's own position within its own set. Nothing reads the result. It reaches no draw, no substitution and no checksum, since the multiplayer synchronization checksum never visits the isometric tile types.

[`NonMarbleMadness`](/keys/nonmarblemadness/) is not the mirror of this key that its name suggests, and does have an effect.

:::caution[An unresolvable number becomes the omission marker]
The pass checks the number against the sets the file actually declares. A number past the last declared set resolves to the no-substitute marker, and so does a number whose set holds fewer tiles than this tile's own position within its set.
:::
