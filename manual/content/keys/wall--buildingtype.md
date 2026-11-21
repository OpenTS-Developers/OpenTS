---
key: Wall
scope: buildingtype
label: Wall structure
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "no"
---

`Wall=yes` makes a BuildingType a wall segment that never stays on the map as a structure. When it is placed, it puts its [`ToOverlay`](/keys/tooverlay/) overlay in the cell and makes the placing house the cell's owner. It then reveals the map within its [sight range](/systems/map-visibility/#sight-range) and deletes itself. If the cell refuses the overlay, the placement fails. [From structure to overlay](/systems/walls-and-gates/#from-structure-to-overlay) covers the cell tests.

Give the type a `ToOverlay` in its `art.ini` entry. Without one, placing the structure crashes the game.

A wall segment may be placed next to any cell its house owns, not only next to a building. This lets a wall run extend from the end of an earlier one. [Placement decision order](/systems/base-adjacency/#placement-decision-order) covers the full test.

Its build time is multiplied by [`WallBuildSpeedCoefficient`](/keys/wallbuildspeedcoefficient/), after every other build-time adjustment. [How long it takes](/systems/production/#how-long-it-takes) lists the steps.

On the sidebar, its cameo sorts into the wall group, with firestorm walls and laser fences.
