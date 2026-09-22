---
title: Base placement and adjacency
summary: "Whether a player may place a building at a site, based on the anchor buildings and owned wall cells near it."
category: buildings-economy
keys:
  - Adjacent
  - BaseNormal
  - BuildOffAllyAnyStructure
related:
  - type: format
    id: spawn-ini
---

A player can place a building only near an **anchor**: a building already on the map that the new one may be built against. The anchor must belong to the player, or to a mutually allied player when the match allows [building off an ally](#building-off-an-ally). Where no anchor is in range, the placement cursor shows the site as blocked and clicking there does not place the building.

The rule applies only to buildings a player places from the sidebar. Computer houses place their buildings without it. An upgrade clicked onto a building that accepts it is also exempt.

Only the placing player's machine runs the check. The other machines in the match accept the placement without repeating it.

## Anchor eligibility

Two keys decide the check. [`BaseNormal`](/keys/basenormal/) belongs to the building already on the map, and [`Adjacent`](/keys/adjacent/) belongs to the building being placed.

| Setting | Read from | What it controls |
| --- | --- | --- |
| [`BaseNormal`](/keys/basenormal/) | The type of a building already on the map | Whether a building of that type may serve as an anchor |
| [`Adjacent`](/keys/adjacent/) | The BuildingType being placed | How far its foundation searches for an anchor |

```ini title="rules.ini"
[GAPOWR]
BaseNormal=no ; a placed GAPOWR cannot anchor later placements
Adjacent=5    ; a GAPOWR being placed searches this far for an anchor
```

`Adjacent` is not a radius around an existing building. Raising it on one BuildingType lets that type be placed farther from its anchor and changes nothing for other types.

Laser fence types anchor like any other building unless their section sets `BaseNormal=no`. Stock rules set it for the fence post `NAPOST` but not for the fence section `NAFNCE`.

## Placement decision order

The search covers the block of cells that extends `Adjacent` + 1 cells beyond each edge of the pending foundation, corners included. Cells inside the foundation itself are not examined. With `Adjacent=5`, for example, the search reaches six cells out from each side. A negative value leaves no cells to examine, so the check always fails.

The adjacency check passes when any examined cell passes one of these tests:

- **Placing any building:** the cell holds an eligible anchor. That is a building whose type has `BaseNormal=yes`, owned by the placing player or by an ally the match admits.
- **Placing a wall:** the cell is owned by the placing player, whether or not a building stands there. A player owns the cells under their walls; [walls and gates](/systems/walls-and-gates/#who-owns-a-wall) explains how wall cells get an owner. This lets a wall run extend from the end of an earlier wall.

If no examined cell passes either test, the check fails and the building cannot be placed there.

:::note[Adjacent zero still permits contact]
Because the search reaches one cell beyond the `Adjacent` value, `Adjacent=0` still finds an anchor that touches the foundation, including diagonally.
:::

## Building off an ally

`BuildOffAlly=yes` in the [launch file](/formats/spawn-ini/) lets a player's placements use a mutually allied player's buildings as anchors. The alliance must run both ways. A one-sided alliance, in either direction, anchors nothing.

An ally's building must still have `BaseNormal=yes`. [`BuildOffAllyAnyStructure=no`](/keys/buildoffallyanystructure/) further limits ally anchors to construction yards. A player's own buildings anchor their placements under either setting.

The wall test is unchanged. It accepts only cells the placing player owns, so a wall run cannot start from an ally's wall.

Like the rest of the check, `BuildOffAlly` affects only human players.
