---
key: IsARock
summary: Overlay that objects on a slope beside it are drawn in front of, and whose own depth is not lifted off the ground.
see_also: [DrawFlat, High]
when_omitted:
  kind: value
  value: "no"
---

The flag does two unrelated things.

In the draw, it cancels the depth bias that [`DrawFlat=no`](/keys/drawflat/) applies, putting the shape back on the ground while leaving the upright gradient it selected in place. On a `DrawFlat=yes` overlay there is no bias to cancel and the flag changes nothing about the draw.

In the depth sorting of everything else, it moves objects standing on sloped ground forward. A vehicle or a soldier on a ramp looks at the cells to its south, east and south-east. If one of them holds an overlay with this flag, the object's depth is nudged forward so that it is drawn in front of the rock rather than buried in it.

```ini title="rules.ini"
[MYBOULDER]      ; example rock overlay beside cliff ramps
DrawFlat=no
IsARock=yes
```

:::caution[The neighbor scan stops at the first cell holding any overlay]
The three cells are tested one after another and each test is only reached when the cell before it has no overlay at all. An ordinary overlay to the south therefore hides a rock to the east or south-east, and the object on the ramp is drawn as though the rock were not there.
:::
