---
key: GDIGateTwo
summary: The BuildingType that joins a brick or sandbag wall running north to south.
see_also: ["system:walls-and-gates", "Gate"]
when_omitted:
  kind: value
  value: none
---

Wall stitching is not a flag on a type. The [connection logic](/systems/walls-and-gates/#connection-frames) accepts the named type as a continuation of a brick or sandbag wall, and only from the north and the south. A wall run therefore reads through the gate along that axis, while one arriving from the east or west stops dead at it. `GAWALL` and `GASAND` are the stock overlays this covers.

The same name also lets the type be placed on a brick or sandbag wall the house already owns, whatever damage that wall has. An ordinary wall building is accepted over such a wall only once it is damaged.

The two cells capping the gate's run are updated both when the type is placed and when it is taken off the map: one cell north of its origin and three cells south of it. Those offsets assume a three-cell footprint.

None of that requires [`Gate=yes`](/keys/gate/). The flag supplies the door cycle and the clearing of walls under the footprint at placement; this key supplies the wall connection, and a type may set either without the other.
