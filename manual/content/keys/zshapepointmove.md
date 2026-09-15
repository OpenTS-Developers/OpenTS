---
key: ZShapePointMove
summary: Moves the shared depth shape stamped under a structure, in screen pixels.
see_also: ["NormalZAdjust", "Foundation"]
when_omitted:
  kind: value
  value: "0,0"
---

Every structure narrower than six cells is drawn together with one shared depth shape, stamped from a fixed point that the structure's own footprint then shifts. It is `BUILDNGZ.SHP`, the shape that supplies the depth other objects sort against. This pair of pixel offsets is added to that point before the footprint shift, moving where the shared shape is sampled from without moving the structure. The first figure moves the sampling point right inside that shape; the second moves it down. The depth pattern that lands under the structure moves the opposite way.

```ini title="art.ini"
[MYREFN] ; example refinery, drawn from its own Image ID
Foundation=4x3
ZShapePointMove=24,-12
```

A structure six or more cells wide is drawn with no depth shape at all, so the offset has nothing to move; the [`6x4`](/keys/foundation/#the-irregular-sizes) footprint is the only one that reaches that width. The fogged copy of a structure applies no width test, so a `6x4` structure does take the shared shape, and this offset with it, for as long as it is drawn fogged. The offset is also computed and then discarded for the frames a [`Gate=yes`](/keys/gate/) structure draws while its gate is open or moving.

:::caution[A value with fewer than two numbers is refused outright]
Both figures are taken from a single value in `x,y` form. A value with one number is rejected, and the offset stays at its default `0,0`; no single figure is applied on its own.
:::
