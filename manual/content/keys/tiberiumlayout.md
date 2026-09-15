---
key: TiberiumLayout
summary: How many tiberium fields a generated map is given, as a figure from 0 to 100.
see_also: [Tiberium, TiberiumWildlife, UseBlueTiberium, NumPlayers]
when_omitted:
  kind: value
  value: "0"
  note: Three fields spread across the map, on top of the field each player start point gets.
---

The field sites and the player start points are chosen together, from one spread of widely separated cells: the start points are taken off the front of that spread and the tiberium fields are what remains. The figure adds one site per ten points, so `0` leaves three fields, `50` leaves eight and `100` leaves thirteen. Only every tenth point changes anything. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
TiberiumLayout=50
Tiberium=60
```

Because the start points come out of the same spread, the field count does not change with the player count: a two-player map and an eight-player map at the same figure are given the same number of fields. What the player count changes is the amount of tiberium in each of them, which [`Tiberium`](/keys/tiberium/#scope-random-map-generation) covers.

Every player also gets a field of their own on top of these, planted at their start point. Those are sized by how far the player sits from the fields already placed, so the player left furthest from the map's tiberium is given the largest one.
