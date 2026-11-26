---
key: YSortAdjust
summary: Biases where the animation falls in the ground layer's drawing order, in leptons.
see_also: ["YDrawOffset", "Surface", "ActiveAnimYSort", "MoveFlash"]
when_omitted:
  kind: value
  value: "0"
---

Moves the animation earlier or later in the ground layer's drawing order. Objects in that layer are drawn in order of their Y coordinate in leptons, and this value is added to the animation's coordinate for that sort; a cell is 256 leptons. A negative value draws the animation behind objects it would otherwise cover, and a positive value draws it in front of them.

Each animation takes the value when it is created. The shipped wake animation sets it:

```ini title="art.ini"
[WAKE2]
Theater=yes
Flat=yes
Surface=yes
Translucent=yes
Rate=120
YSortAdjust=-64
```

The value moves nothing on screen: the artwork is drawn in the same place, and only what covers what changes. [`YDrawOffset`](/keys/ydrawoffset/) moves the artwork.

Only the ground layer is sorted, so the value affects only an animation that has [`Surface=yes`](/keys/surface/) or is attached to an object. Every other animation is in the air layer, where the value has no effect. A burning structure's fire is attached to the structure, and a weapon's firing animation to the vehicle, infantry or aircraft that fired it. A structure's firing animation is not attached.

An animation that a structure runs in one of its animation slots uses the slot's value instead, which is `0` when the slot sets none. [Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers that value and its range. The type's own value has no such range limit.

The marker of a move order, set by [`MoveFlash=`](/keys/moveflash/), is not sorted with the other objects, so the value does not move it. Other animations of the same type use the value as usual.
