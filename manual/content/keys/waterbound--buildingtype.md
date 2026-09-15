---
key: WaterBound
scope: buildingtype
label: Water-bound structure
see_also: [Buildable, PlaceAnywhere, "system:base-adjacency"]
when_omitted:
  kind: value
  value: "no"
---

The setting picks which of two terrain tests is taken over each cell of the foundation, and every placement check reads the same choice. That covers the legality test the structure is put through as it arrives, the one a vehicle runs before deploying into it, and the cursor a player holds while placing it.

| Setting | What each foundation cell must satisfy |
| --- | --- |
| `WaterBound=no` | Not a bridge, not a cell under a bridge, no ramp, and a land type with [`Buildable=yes`](/keys/buildable/) |
| `WaterBound=yes` | A land type the floating movement kind can cross at all: a `Float=` cost above zero, set alongside `Buildable=` in that land type's own section |

Everything the two tests share still applies either way: the cell must be empty, inside the local radar bounds and free of overlay. What the water-bound branch drops is the bridge, under-bridge and ramp rejection, which is tested only on the land-bound side.

:::note[There is no partial answer]
The choice is one test or the other, not a widening: the land-bound conditions are not applied to a water-bound structure, and `Buildable=` stops being read for it entirely. Which ground a water-bound structure may stand on is decided by which land types have a `Float=` cost and by nothing else.
:::
