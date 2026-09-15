---
key: DirtRoadStraight
summary: Parsed tile set of straight dirt road pieces that nothing reads.
no_effect: true
see_also: [DirtRoadCurve, DirtRoadJunction, DirtRoadSlopes]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The resolved set reaches exactly one test, which asks whether a cell has a dirt road. Nothing calls it. The straight runs are still laid and still connect, but they are found inside the 101-tile run counted from [`DirtRoadCurve`](/keys/dirtroadcurve/) rather than from this role.
