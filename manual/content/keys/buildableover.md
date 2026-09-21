---
key: BuildableOver
summary: Lets structures be placed on cells that hold this overlay.
see_also: [Buildable, Land, Wall, "system:base-adjacency"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MYRUBBLE] ; an OverlayType registered in [OverlayTypes]
Land=Clear
BuildableOver=yes
```

By default, an overlay blocks structure placement on its cell. With `BuildableOver=yes` it does not, and the cell's land type decides instead.

That land type is the overlay's own [`Land`](/keys/land/), or the tile's land type if the overlay sets [`NoUseTileLandType=no`](/keys/nousetilelandtype/). Set `Land` to a land type whose [`Buildable`](/keys/buildable/) is `yes`. Bridge cells and sloped cells are refused whatever the land type.

The flag has no effect on a [`Wall=yes`](/keys/wall/#scope-overlaytype) overlay, which blocks placement as it would without the flag.

The flag belongs to the overlay, so it applies to every structure type, whether a player places it or a computer house builds its base or its walls.

Placing a structure does not remove the overlay, so the overlay is still there after the structure is gone.
