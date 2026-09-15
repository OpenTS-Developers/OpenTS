---
key: TerrainPalette
summary: Draws the structure with the cell's terrain colors instead of its owner's.
see_also: ["ExtraLight", "Remapable"]
when_omitted:
  kind: value
  value: "no"
---

An object is normally drawn through the color table belonging to its owning house, which is what gives a [`Remapable=yes`](/keys/remapable/) structure its owner's color. With this flag set, the structure is drawn through the color table its cell uses for terrain. The brightness used is the cell's tile lighting rather than the level worked out for the object, so [`ExtraLight`](/keys/extralight/) does not reach it while it is drawn normally. Only structures read this flag; terrain objects such as trees and rocks do not.

```ini title="art.ini"
[MYTREEHOUSE] ; example civilian structure, drawn from its own Image ID
TerrainPalette=yes
```

A cell that has no table yet is given one as the structure is drawn. If one still cannot be built, the structure is not drawn at all that pass. The copy drawn under fog takes the cell's table as well, so a flagged structure keeps its terrain colors while fogged; the palette is the only part of the draw the flag decides there.
