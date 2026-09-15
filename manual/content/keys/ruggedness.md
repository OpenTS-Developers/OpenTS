---
key: Ruggedness
summary: How hilly a generated map's ground is, as a figure from 0 to 100.
see_also: [RegionSize, Accessibility]
when_omitted:
  kind: value
  value: "0"
  note: The hill walk is skipped, and the ground keeps exactly what the region pass left it.
---

The hills are laid down by a walk across the map that gives each cell a height drawn from the neighbors already settled. That is what makes the ground roll rather than step. The figure sets both how far each step of that walk may move (a tenth of a level plus a thousandth per point) and how widely the walk is allowed to wander around it. Cells beside a shoreline or a cliff face are pinned first and keep the height they were given. The walk therefore cannot raise a hill into the sea or bury an existing cliff. [Map seed files](/formats/map-seed/) covers the section it is written in.

```ini title="map seed file"
[RandomMap]
Ruggedness=40
```

The walk is skipped entirely unless the figure is at least `2`, so `0` and `1` both leave the ground exactly as the region pass left it. This is separate from the cliffs: [`RegionSize`](/keys/regionsize/) governs the sharp height changes between regions, and this figure the gentle relief laid over them afterward.

The figure is held to `0` through `100` only on [the dialog path](/systems/map-generation/#the-dialog-path-and-the-scenario-path). A figure written by hand in a seed file reaches the generator as it stands. Below `2` the walk is skipped anyway, and above `100` it only raises how far and how widely each step may move.
