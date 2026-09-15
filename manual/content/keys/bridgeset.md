---
key: BridgeSet
summary: The tile set that supplies the sixteen pieces a road bridge is built from.
see_also: [TrainBridgeSet, BridgeMiddle1, BridgeStrength]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The value is a tile-set number, resolved into the index of that set's first tile while [the theater control file](/formats/theater-control/) is read. Every road-bridge test then works in offsets from that index. The position of a cell's piece is its tile index minus the resolved index plus one. The ten `Bridge` position keys name which of the sixteen positions holds which piece.

```ini title="TEMPERAT.INI"
[General]
BridgeSet = 19          ; road bridge pieces come from [TileSet0019]
BridgeTopLeft1 = 1      ; the first piece of that set
BridgeTopLeft2 = 2
BridgeBottomRight1 = 3
BridgeBottomRight2 = 3  ; a pair may name the same piece
BridgeTopRight1 = 4
BridgeTopRight2 = 5
BridgeBottomLeft1 = 6
BridgeBottomLeft2 = 6
BridgeMiddle1 = 7       ; positions 7 through 11
BridgeMiddle2 = 12      ; positions 12 through 16
```

Two things beyond artwork follow from a cell answering as a road bridge. Its span is registered as a crossing between the two banks, which is how a route is found from one side to the other over ground that is otherwise unconnected. The same crossing list includes the map's tunnels. Tunnel pieces come from their own control-file roles, not from this set. A cell hit by combat damage is offered to the road-bridge handler before the [rail one](/keys/trainbridgeset/). The road set is what decides which of the two brings a span down.

:::caution[An unresolved set claims the theater's first tiles]
Nothing guards the road-bridge test against a role that no tile set answered. Left unresolved, the test accepts every tile index from `0` through `14`. The first fifteen tiles the theater loads then answer as road bridge pieces wherever it is asked.
:::
