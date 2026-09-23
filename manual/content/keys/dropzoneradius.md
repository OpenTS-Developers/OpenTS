---
key: DropZoneRadius
summary: How far around a drop zone flare the map is revealed, in cells.
see_also: [DropZoneAnim, "system:map-visibility"]
when_omitted:
  kind: value
  value: "4"
  note: 1024 leptons.
---

```ini title="rules.ini"
[AudioVisual]
DropZoneRadius=6
```

The reveal happens once, on the frame a [`DropZoneAnim`](/keys/dropzoneanim/) animation is created, for every human player, whichever house's trigger dropped the flare.

The figure is written in cells and stored at 256 leptons to the cell. The reveal divides it back down to whole cells and discards the remainder. Fractions are accepted on the way in but have no effect on the outcome unless they take the value across a whole cell. Anything below `1` leaves a radius of nothing.

:::caution[A value of exactly `-1` is read as no value at all]
The read asks for the figure with `-1` as its own marker for a missing entry and cannot tell that answer apart from a real one. `DropZoneRadius=-1` therefore leaves whatever an earlier rules layer set. Every other negative value is stored and then divided down to a negative cell count.
:::
