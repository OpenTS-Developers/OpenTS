---
key: DrawFlat
summary: Overlay artwork whose depth is written as a marking on the ground rather than as something standing upright.
see_also: [IsARock, Wall, Tiberium, High]
when_omitted:
  kind: value
  value: "yes"
---

Turning the flag off changes two things about how the shape is written into the depth buffer. The depth gradient across the artwork becomes the upright one instead of the ground one. The shape sorts against passing objects as a thing standing in the cell rather than as a stain on it. The whole shape is also biased by twelve, the step the depth uses for one terrain height level. It sorts as though it stood a level above the ground it is on. The screen position of the artwork is not affected either way.

```ini title="rules.ini"
[MYFENCE]        ; example upright ground fixture that is not a wall
DrawFlat=no
```

[`IsARock`](/keys/isarock/) removes the bias again while leaving the upright gradient in place.

:::note[Several kinds of overlay never reach this decision]
The overlay draw picks a path by what the overlay is before it looks at this flag. A bridge deck, a Tiberium overlay, a [`Wall=yes`](/keys/wall/#scope-overlaytype) overlay and the vein carpet each draw their own way with a fixed gradient. The flag governs only the overlays that are none of those. Walls in particular always use the upright gradient whatever this key says.
:::
