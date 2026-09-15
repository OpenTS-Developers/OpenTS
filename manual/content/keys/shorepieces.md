---
key: ShorePieces
summary: Forty-two-tile set of the shoreline pieces laid between water and land.
see_also: [WaterSet]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is selected, so the role stays unresolved.
---

A cell holding any of the forty-two is shore, which makes it count as holding water without being open [`WaterSet`](/keys/waterset/) water. A transport standing on one refuses to take on a passenger. A cell holding a set's first ice tile with a shore cell on any of its four sides is pushed off solid ice onto the first edge tile of [`Ice1Set`](/keys/ice1set/), whichever of the three sets it belonged to.

Shore is not a blend family. Where a LAT family derives its piece from a four-bit pattern of matching neighbors, the [random map generator](/formats/map-seed/) picks a shore piece from the pattern of water across all eight neighbors. Each of the forty-two has three fixed properties of its own: the cell offset the piece anchors at relative to the cell being grown, an interchangeability group, and a facing. Two tests keep a generated coastline continuous rather than a ring of mismatched corners, and which one runs depends on the cell a piece would overlap. Where that cell belongs to another region, the two groups must agree and the existing tile is kept. Where it belongs to the same region, the pairing is refused when the two facings stand three to five steps apart.

:::danger[The shore builder indexes the tile heap without checking]
When the random map generator places a piece it looks the tile up directly by this role's index plus the piece number, with no test that the role resolved. A theater that omits the key therefore has the generator read one entry below the start of the tile heap whenever the neighbor pattern selects the set's first piece. Every other piece indexes into the theater's own early tiles. That out-of-range entry becomes the pending placement object and is dereferenced as one. Random maps are stored as a seed and rebuilt by every player at load time, so the fault reaches each client rather than only the machine that generated the map.
:::
