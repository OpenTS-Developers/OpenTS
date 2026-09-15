---
key: Height
scope: buildingtype
label: Structure vertical extent
see_also: ["Foundation", "MidPoint"]
when_omitted:
  kind: value
  value: "1"
---

The figure is a count of 200-lepton steps. The engine's own terrain height level is 104 leptons, so this is not that unit. It is read as a whole number, so a value such as `1.5` in an art entry stores `1`.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
Foundation=4x3
Height=2
```

Three things read it, and none of them is the drawing of the structure itself:

- The selection brackets and the health pips are laid out from the structure's dimensions, so the figure decides how tall the bracket drawn around a selected structure rises.
- A jumpjet works out the level it must fly at from the height of whatever occupies a cell, and a structure reports 200 leptons a step as its bulk there. A moving jumpjet takes the cell ahead's figure only when that is the larger of the two, and otherwise the average of the cell it is over and the cell ahead.
- A non-arcing, non-voxel shot at a structure more than 200 leptons above or below the firing turret takes its aim point from this figure rather than from where the shot was aimed. The projectile is pitched at the top of the structure instead of at its base. That aim point is an absolute height above world zero and has no term for the ground the structure stands on, so a structure on raised terrain is aimed at below its own roof.
