---
key: AlphaImage
scope: aircrafttype
label: Object light shape
see_also: ["Image"]
when_omitted:
  kind: value
  value: ""
  note: No light shape is loaded and the object contributes nothing to the lighting pass.
---

The value is a filename without its extension, and only a non-empty value is taken. The named `<value>.SHP` is loaded once with the rules and attached to each object of the type as it is placed on the map. The shape is centered on the object, and its pixels are blended into the lighting buffer, brightening whatever is drawn underneath. The light is removed with the object it belongs to.

```ini title="rules.ini"
[MYLAMP] ; a BuildingType
AlphaImage=MYGLOW ; MYGLOW.SHP brightens the ground around it
```

Unlike the object's own artwork, this shape is not selected through the [Image ID](/keys/image/); it is named here directly.

:::caution[The light does not travel with a moving object]
The shape's position is fixed where the object enters play and is never recalculated, so a vehicle drives out from under its own light and leaves it behind on the cell it started from.
:::
