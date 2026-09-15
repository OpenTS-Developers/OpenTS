---
key: DirtRoadJunction
summary: The tile set that supplies the eleven dirt road junctions.
see_also: [DirtRoadCurve, DirtRoadStraight, DirtRoadSlopes]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

A rural road network opens on a junction, and this role is where the [random map generator](/formats/map-seed/) finds one. All eleven shapes counted from it are tried once at the starting cell, beginning at a randomly chosen shape so that networks do not all open the same way. The network is abandoned outright when none of the eleven fits.

The eleven also fall inside the run of 101 tiles that [`DirtRoadCurve`](/keys/dirtroadcurve/) fixes, since that run's length is the curve, junction and straight counts added together. The two roles therefore describe overlapping ground, and they agree only while the junction role resolves to tiles inside the run counted from [`DirtRoadCurve`](/keys/dirtroadcurve/). A junction laid outside that run is matched back to the library's first entry, and the generator extends the road from that entry's connection data instead.
