---
key: GDIBarracks
summary: Gives the structure a preferred exit cell one east and two south of its own, placed at the door coordinate.
see_also: [NODBarracks, ExitCoord, "system:production"]
when_omitted:
  kind: value
  value: "no"
---

Whenever the structure needs a cell to send an object out to, the search tries the cell one east and two south of the first cell of its foundation before anything else. That cell is taken if it lies inside the map's radar bounds and the object could enter it; otherwise the search falls through to the ring of cells the foundation's own size supplies.

Taking that cell also moves where the object appears. The object is always put down at a coordinate pulled back inside the foundation (one cell in along each axis on which the chosen cell falls outside it) and then ordered to walk onto the chosen cell. When the chosen cell is this door cell, [`ExitCoord`](/keys/exitcoord/) is added to that placement coordinate, which is how the object comes out standing in a doorway rather than on the middle of a cell. Every other exit cell ignores `ExitCoord` entirely.

The structure is recognized by the flag alone; no BuildingType ID is special-cased, and nothing here decides what the structure produces or satisfies a `BARRACKS` prerequisite. [Leaving the factory](/systems/production/#leaving-the-factory) covers which structures reach this search at all. A [`WeaponsFactory=yes`](/keys/weaponsfactory/) structure, a refinery and a weeder each send finished objects out by a path of their own that never reads it, while a repair bay releasing a repaired vehicle does.

:::note[Both barracks flags may sit on one structure]
The two are tested one after the other, this one first. A structure with both prefers this door cell and falls back to the [`NODBarracks=yes`](/keys/nodbarracks/) cell one further east, and `ExitCoord` is applied to whichever of the two was taken.
:::
