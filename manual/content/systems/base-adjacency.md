---
title: Base placement and adjacency
summary: "Determines whether a pending building placement is allowed, from the eligible buildings and owned cells around it."
category: buildings-economy
keys:
  - Adjacent
  - BaseNormal
  - BuildOffAllyAnyStructure
related:
  - type: format
    id: spawn-ini
---

The proximity check runs for the local player's own house only. It applies to a pending BuildingType placement with a valid foundation, and a placement for any other house passes without a scan. The check looks at the cells around that foundation for an anchor: a building already on the map that the placement may be built against. An anchor must belong to the same house, or to a mutually allied one when the match allows it.

## Anchor eligibility

Two settings on the object types decide the check, and the table sets each against the object it is read from: one comes from what already stands on the map, the other from what is being placed. Neither is a radius the existing building projects, so changing `Adjacent` on a BuildingType moves where that type may be placed and leaves every other type where it was.

| Setting | Read from | What it controls |
| --- | --- | --- |
| [`BaseNormal`](/keys/basenormal/) | The type of a building already on the map | Whether a building of that type may serve as an anchor |
| [`Adjacent`](/keys/adjacent/) | The BuildingType being placed | How far its pending foundation searches for an anchor |

```ini title="rules.ini"
[GAPOWR]
BaseNormal=no ; placed instances cannot anchor later placements
Adjacent=5   ; pending instances use this search distance
```

Two building types can never anchor, whatever the assignment says: a type whose ObjectType ID is `NAFNCE` or `NAPOST` has `BaseNormal` forced to no.

## Placement decision order

1. Take the pending BuildingType's foundation dimensions, and add one cell to its `Adjacent` value on every side to form a rectangular scan area.
2. Skip cells covered by the pending foundation.
3. Accept a wall placement when a scanned cell belongs to the same house, whether or not a building stands in that cell. A house owns the cells its own walls stand on.
4. Accept any placement, a wall's included, when a scanned cell holds an eligible anchor: a building whose type has `BaseNormal=yes`, owned by the same house or by an ally the match admits.
5. Reject the placement when no scanned cell satisfies either test.

## Building off an ally

A match may admit a mutually allied house's buildings as anchors alongside your own. The alliance must run both ways: a house that has allied you but has not been allied in return anchors nothing. The anchor still needs `BaseNormal=yes`, and step 3 is untouched, so an ally's walls and bibs open no ground. Setting [`BuildOffAllyAnyStructure=no`](/keys/buildoffallyanystructure/) narrows which of the ally's buildings count, from any of them to construction yards alone.

Only the placing machine runs the check; nothing re-tests adjacency when the placement reaches the others. A computer house builds its base by a rule of its own that never looks at an ally, so the option reaches human placement alone. A [launch file](/formats/spawn-ini/) sets it.

:::note[Adjacent zero still permits contact]
The scan adds one cell to the stored value. `Adjacent=0` can therefore find an eligible anchor touching the pending foundation.
:::
