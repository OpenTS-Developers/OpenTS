---
key: ExtraLight
summary: The lighting offset applied to the structure's own artwork.
see_also: ["TerrainPalette", "BibShape", "UnderDoorAnim"]
when_omitted:
  kind: value
  value: "0"
---

The figure is added to the lighting level of the cell the structure stands on, on the scale where 1000 is the unmodified level. `ExtraLight=-100` therefore draws the structure a tenth of full brightness darker than the ground beneath it. A positive figure brightens the structure above the cell's level the same way. The figure itself has no cap, but each color stops at its brightest value, so an extreme figure saturates the artwork rather than brightening it further.

```ini title="art.ini"
[MYICBM] ; example missile launcher, drawn from its own Image ID
ExtraLight=-100
```

It reaches the structure's main shape, its [`BibShape`](/keys/bibshape/) apron and its [`UnderDoorAnim`](/keys/underdooranim/), and the copy of the structure drawn under fog. It does not reach the [`DoorAnim`](/keys/dooranim/) frames, which are drawn at the cell's own level, nor the door frames of a [`Gate=yes`](/keys/gate/) structure. Any attached animation is a separate object with its own lighting.

A [`TerrainPalette=yes`](/keys/terrainpalette/) structure takes its brightness from the cell's tile lighting instead while it is drawn normally, and this figure does not reach it. Its fogged copy is not drawn that way and keeps the figure.
