---
key: CraterLevel
summary: How many cells around a meteor impact are slumped into a crater.
see_also: [IsMeteor]
when_omitted:
  kind: value
  value: "4"
---

The value is a count of cells rather than a depth. It selects how far the deformation spreads from the impact cell:

| Value | Cells deformed |
| --- | --- |
| `0` | none |
| `1` | the impact cell |
| `2` | the impact cell and its four corner neighbors (north-east, south-east, south-west and north-west) |
| `3` | the impact cell and all eight neighbors |
| `4` or more | as `3`, and the impact cell a second time |

The setting is read only at the impact of an animation or voxel animation declaring [`IsMeteor=yes`](/keys/ismeteor/), and an impact landing on a bridge deck deforms nothing at all. A warhead that reshapes the ground works from its own [`Deform`](/keys/deform/) and [`DeformThreshhold`](/keys/deformthreshhold/) and always takes the single cell it lands on.

Each cell is treated on its own: one that does not have all eight of its own neighbors inside the playfield refuses the deformation, so a crater near the edge of the playfield comes out clipped.

:::caution[Only the last cell of a burst caves in completely]
A cell's crater arrives in two halves: one randomly chosen corner slumps at once, and the other three follow five frames later. Only one deferred half is held at a time, and every further cell in the same burst that actually deforms overwrites it. At `2` or more, therefore, all but the last cell touched keep three of their four corners.
:::
