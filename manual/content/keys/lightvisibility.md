---
key: LightVisibility
summary: The radius, in leptons, that a structure's glow reaches.
see_also: [LightIntensity, LightRedTint, LightGreenTint, LightBlueTint]
when_omitted:
  kind: value
  value: "5000"
---

```ini title="rules.ini"
[MYLAMP] ; example light post BuildingType
LightVisibility=2560  ; ten cells at 256 leptons each
```

The figure is a raw lepton count rather than a distance in cells: 256 leptons to the cell, so the built-in `5000` reaches a little over nineteen cells. A cell inside the radius takes a share of [`LightIntensity`](/keys/lightintensity/) and of each of the three tints, full at the structure's own center and falling in a straight line to nothing at the radius itself. Widening the radius therefore both spreads the glow and flattens its falloff, so cells that were already lit are lit more strongly than before.

The radius is read only when the intensity is not zero, since a structure with no intensity has no light source for it to size.
