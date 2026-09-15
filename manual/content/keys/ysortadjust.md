---
key: YSortAdjust
summary: Biases where the animation falls in the ground layer's drawing order, in leptons.
see_also: ["YDrawOffset", "Surface", "ActiveAnimYSort", "MoveFlash"]
when_omitted:
  kind: value
  value: "0"
---

The value is copied to each animation of the type when the animation is created. It is added to the animation's sorting position, which decides the order the ground layer is drawn in. The position is the animation's Y coordinate, measured in leptons; a cell is 256 leptons. A negative figure sorts the animation earlier and so puts it behind the objects it shares the layer with; a positive one puts it in front.

The shipped wake animation sets the figure in art.ini:

```ini title="art.ini"
[WAKE2]
Theater=yes
Flat=yes
Surface=yes
Translucent=yes
Rate=120
YSortAdjust=-64
```

It moves nothing on screen. The artwork is drawn in the same place either way, and only what covers what changes. [`YDrawOffset`](/keys/ydrawoffset/) is the setting that moves the sprite.

The ground layer is the only layer kept in order, so an animation that is neither [`Surface=yes`](/keys/surface/) nor attached to an object is in the air layer instead. A burning structure's fire is attached to its structure, and a weapon's firing animation is attached to the object that fired it. A structure firing a weapon leaves the animation unattached. The air layer is not sorted, and the figure does nothing for it.

An animation a structure runs in one of its slots receives the slot's own figure instead of this one. That applies even when the slot states nothing, because the slot's figure is then zero. [Placement and draw order](/systems/building-animations/#placement-and-draw-order) covers that override and the byte the slot's figure has to fit in. The type's own figure has no such limit.

:::caution[The move flash animation loses its figure in multiplayer]
The first time a move order is given in a network game, the figure of the animation named by [`MoveFlash=`](/keys/moveflash/) is overwritten with -5000. The type keeps that value for the rest of the session, so whatever the type states is discarded there. A campaign and a skirmish leave it alone.
:::
