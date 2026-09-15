---
key: TrainBridgeSet
summary: The tile set that supplies the sixteen pieces a railway bridge is built from.
see_also: [BridgeSet, BridgeRepairHut, "system:capture"]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The set is laid out exactly like the road one and is read through the same ten `Bridge` position keys; [`BridgeSet`](/keys/bridgeset/) covers how a position is turned into a tile.

This set alone decides which kind of span an engineer rebuilds. On arrival at a [`BridgeRepairHut=yes`](/keys/bridgerepairhut/) structure the five-by-five block of cells centered on the soldier is searched, and one tile of this set anywhere inside it selects the rail repair; the road repair runs otherwise. [Repairing a bridge](/systems/capture/#repairing-a-bridge) covers the rest of that arrival.

Combat damage takes the sets in the other order. A struck cell with a low bridge overlay is handled as a low bridge, and neither set is read. Otherwise the road set is tested first. The cell reaches the rail handler only once it has failed both the road middle-piece test and the road deck test that [`BridgeMiddle1`](/keys/bridgemiddle1/) covers.

:::caution[An unresolved set claims the theater's first tiles]
The railway-bridge test has no guard against a role that no tile set answered. Left unresolved, it accepts every tile index from `0` through `14`, so the first fifteen tiles the theater loads answer as railway bridge pieces wherever it is asked.
:::
