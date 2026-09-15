---
key: DirtRoadCurve
summary: The tile set the whole run of 101 flat dirt road pieces is counted from.
see_also: [DirtRoadJunction, DirtRoadStraight, DirtRoadSlopes]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The [random map generator](/formats/map-seed/) holds a fixed library describing, for each dirt road piece, which cells its roads leave from and in which directions. The library is bound to the theater by counting 101 consecutive tiles from this role's first tile. A piece the generator has just laid is matched back to its entry by the same subtraction, so this one role fixes where every flat dirt road piece is found. The eight ramp pieces are appended afterwards from [`DirtRoadSlopes`](/keys/dirtroadslopes/).

The library's length is the curve, junction and straight counts added together. Junctions are also addressed on their own through [`DirtRoadJunction`](/keys/dirtroadjunction/); straights are not addressed on their own at all, since [`DirtRoadStraight`](/keys/dirtroadstraight/) reaches only a test nothing calls.
