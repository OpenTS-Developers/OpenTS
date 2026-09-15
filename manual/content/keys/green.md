---
key: Green
summary: The scenario's green palette tint, where 1 leaves the green channel unchanged.
see_also: [Red, Blue, IonGreen, "system:ion-storms"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="map file"
[Lighting]
Green=.9
```

The value is stored in hundredths and tints the terrain palette that every cell draws its ground through, from the moment the map loads. House color schemes are never tinted by these keys; only an ion storm retints them. The engine reads this value before its ion counterpart in the same section, and [`IonGreen`](/keys/iongreen/) falls back to it.
