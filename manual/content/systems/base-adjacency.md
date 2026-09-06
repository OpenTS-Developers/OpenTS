---
title: Base placement and adjacency
summary: "Determines whether a pending building placement is adjacent to an eligible owned building."
category: buildings-economy
keys:
  - Adjacent
  - BaseNormal
  - BuildOffAllyAnyStructure
related:
  - type: format
    id: spawn-ini
---

The proximity check applies to a pending BuildingType placement with a valid foundation, and only to the local player's own house. It scans the cells around that foundation for an eligible anchor owned by the same house, or by a mutually allied one when the match allows it; a placement for any other house is not scanned at all and passes. Nor is one scanned in the map editor, where the check passes for every house, the local player's included.

## Anchor eligibility

Two settings decide the check, and the table sets them against the object each is read from: one comes from what already stands on the map, the other from what is being placed. Neither is a radius the existing building projects, so changing `Adjacent` on a BuildingType moves where that type may be placed and leaves every other type where it was.

| Setting | Read from | What it controls |
| --- | --- | --- |
| [`BaseNormal`](/keys/basenormal/) | The type of a building already on the map | Whether a building of that type may serve as an anchor |
| [`Adjacent`](/keys/adjacent/) | The BuildingType being placed | How far its pending foundation searches for an anchor |

```ini title="rules.ini"
[GAPOWR]
BaseNormal=no ; placed instances cannot anchor later placements
Adjacent=5   ; pending instances use this search distance
```

## Placement decision order

1. Read the pending BuildingType's foundation dimensions and add one cell to its `Adjacent` value to form the scan area.
2. Skip cells covered by the pending foundation.
3. Accept a wall placement when a scanned cell is owned by the same house, whether or not a building stands in that cell.
4. Accept any placement, a wall's included, when a scanned cell holds an eligible anchor: a building whose type has `BaseNormal=yes`, owned by the same house or by an ally the match admits.
5. Reject the placement when no scanned cell satisfies either test.

## Building off an ally

A match may admit a mutually allied house's buildings as anchors alongside your own. The alliance must run both ways: a house that has allied you but has not been allied in return anchors nothing. The anchor still needs `BaseNormal=yes`, and step 3 is untouched, so an ally's walls and bibs open no ground. [`BuildOffAllyAnyStructure`](/keys/buildoffallyanystructure/) narrows which of the ally's buildings count, from any of them to construction yards alone.

Only the placing machine runs the check; nothing re-tests adjacency when the placement reaches the others. A computer house builds its base by a rule of its own that never looks at an ally, so the option reaches human placement alone. A [launch file](/formats/spawn-ini/) carries it.

:::note[Adjacent zero still permits contact]
The scan adds one cell to the stored value. `Adjacent=0` can therefore find an eligible anchor touching the pending foundation.
:::
