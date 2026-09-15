---
key: LastTilesInSet
summary: Number of tiles the set held when the maps in circulation were numbered, so that older tile indices can be shifted into line.
see_also: [TilesInSet]
when_omitted:
  kind: value
  value: "-1"
  note: No shift is recorded for the set, and tile indices read from a map pass through unchanged.
---

A map stores each cell's terrain as a tile index into the theater heap, so inserting tiles into a set already released would move every index above it and repaint every older map. This key is the record of that insertion. It states the count the set used to have; [`TilesInSet`](/keys/tilesinset/) states the count it has now, and the difference between them is how far the indices above the set have moved.

```ini title="TEMPERAT.INI"
[TileSet0042]      ; example set that gained four tiles after release
SetName=Shore pieces
FileName=SHORE
TilesInSet=12      ; twelve tiles are loaded
LastTilesInSet=8   ; maps were numbered when there were eight
```

A shift is recorded only when the two counts differ. Every tile index a map supplies is then run through the recorded shifts in set order. A shift whose insertion point lies at or below that index adds its difference, so an index written against the older numbering arrives at the tile the theater now holds in that place. Indices below the first insertion point are left alone.

The insertion points are counted in the *old* numbering: a set that records a shift contributes its old count rather than its new one to the running total. A chain of shifts stays consistent as long as every set that grew declares what it grew from.

:::caution[The two counts must both be truthful]
A set that grew but leaves this key out shifts nothing, and every map drawn before it grew reads the wrong tile from that set onward. Declaring a figure larger than the current count records a backward shift instead, which walks older indices down into the set below.
:::
