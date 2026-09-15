---
key: Red
summary: The scenario's red palette tint, where 1 leaves the red channel unchanged.
see_also: [Green, Blue, IonRed, "system:ion-storms"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="map file"
[Lighting]
Red=1
Green=.9
Blue=.8
```

The value multiplies the red component of every terrain palette color, each on a 0-255 scale, and clips the result at 255. Above 1 adds red, below 1 takes it away, 0 removes it entirely, and values clamp to the range 0 through 2. It is stored as whole hundredths, truncated rather than rounded, so Red=.925 runs as .92. The tint is built into the palette every cell draws its ground through as the map loads, so it is in place from the first frame rather than fading in. House color schemes are never tinted by these keys; only an ion storm retints them.

This value is read before its ion counterpart in the same section, and [`IonRed`](/keys/ionred/) falls back to it. A map that gives only the three ordinary tint keys keeps its terrain coloring through [an ion storm](/systems/ion-storms/#lighting). Unit and building art picks the same tint up for the storm's duration.
