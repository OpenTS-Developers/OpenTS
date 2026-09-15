---
key: CellAnim
summary: Animation created on a cell when this overlay is laid on it.
see_also: ["system:tiberium", Overrides, Tiberium]
when_omitted:
  kind: value
  value: none
---

An animation of the named type is created for the cell each time an overlay of the type is laid down, placed at ground level. The overlay does not need [`Tiberium=yes`](/keys/tiberium/#scope-overlaytype): a plain decorative overlay that names an animation here gets one all the same. Where the cell already grows Tiberium, the animation is switched to that crop's color scheme and takes the cell's own brightness. An effect over a standing crop therefore tints with it.

```ini title="rules.ini"
[MYCRYSTAL]           ; example Tiberium overlay
Tiberium=yes
CellAnim=MYSPARKLE    ; example AnimType registered in rules.ini
```

The animation is created whether or not the overlay was actually accepted onto the cell: a placement refused by [`Overrides`](/keys/overrides/) on the overlay already there still produces one.

The setting also stands in for the overlay's own artwork in two places:

- an overlay named by a map's overlay pack is created when it has artwork **or** an animation named here, so an artless type can still be placed from a map;
- radar and map-preview colors come from the animation's artwork when the overlay has no shape of its own. On the in-game radar a Tiberium overlay takes its color from the animation whether or not it has one.

What ends the animation is the animation's own flag, not the overlay's. An animation flagged as animated Tiberium checks each frame that the overlay beneath it still names it here, and marks itself for deletion the moment that stops being true. That is how the effect goes away with the crop. An animation without that flag simply plays out its loop count and ends like any animation, whether or not the overlay is still there.
