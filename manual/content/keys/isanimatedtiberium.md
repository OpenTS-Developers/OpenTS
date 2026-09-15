---
key: IsAnimatedTiberium
summary: Removes the animation as soon as the overlay that placed it stops asking for it.
see_also: ["CellAnim", "system:tiberium"]
when_omitted:
  kind: value
  value: "no"
---

An overlay type that names a [`CellAnim`](/keys/cellanim/) gets one of these animations created a cell and a half along each axis from it. That is where the artwork has to sit for a field-dressing effect to line up. On every frame after any creation delay, an animation with this flag reverses that offset to find the cell it belongs to. It marks itself for deletion the moment that cell holds no overlay, or holds one that no longer names this animation type. The effect therefore goes when the growth beneath it is harvested, built over, or blown up, without anything having to hunt the animation down.

The flag does nothing else. It does not place the animation, choose its artwork, or color it. An overlay that hands out a cell animation colors it from the Tiberium in its own cell at the moment it is placed.

:::caution[A restored game recolors from the wrong cell]
The color is not saved with the animation and is looked up again when a saved game is loaded, but from the cell the animation stands in rather than from the cell that placed it. The two are never the same cell. An animation whose own cell holds no Tiberium comes back drawn in the colors an animation of its type would ordinarily get.
:::

```ini title="art.ini"
[BIGBLUE] ; the animation the large Tiberium overlays name as their CellAnim
IsAnimatedTiberium=yes
Surface=yes
```
