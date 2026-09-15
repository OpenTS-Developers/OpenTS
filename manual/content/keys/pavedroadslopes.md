---
key: PavedRoadSlopes
summary: Tile set of paved road ramp pieces that nothing reads.
no_effect: true
see_also: [PavedRoads, DirtRoadSlopes]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

The resolved set reaches exactly one test, which asks whether a cell has a paved road slope, and nothing calls it. A road climbing a ramp is drawn from these pieces and treated as whatever their artwork reports; the role changes none of that. [`DirtRoadSlopes`](/keys/dirtroadslopes/) is the dirt counterpart.
