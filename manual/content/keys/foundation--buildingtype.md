---
key: Foundation
scope: buildingtype
label: Structure footprint
see_also: ["Bib", "BibShape", "Adjacent", "system:base-adjacency", "system:production"]
when_omitted:
  kind: value
  value: "1x1"
---

The value is one of 22 fixed size names, matched without regard to case. Twenty-one are `WxH` spellings (`1x1`, `2x1`, `1x2`, `2x2`, `2x3`, `3x2`, `3x3`, `3x5`, `4x2`, `1x3`, `3x1`, `4x3`, `1x4`, `1x5`, `2x6`, `2x5`, `5x3`, `4x4`, `3x4`, `6x4` and `0x0`), and the twenty-second is `3x3Refinery`. Anything else is not a size at all and resolves to `1x1`.

The name is the only thing that fixes a structure's shape on the map. From it come the list of cells the structure occupies and marks as its own, the ring of cells a finished object is sent out to, and the width and height every other rule asks for. It also fixes the diamond the structure is stamped as on the radar. The occupied cells are impassable to ground units; a gate's footprint is the exception, passable while the gate stands open.

```ini title="art.ini"
[MYWEAP] ; example war factory, drawn from its own Image ID
Foundation=4x3
```

The width and height are the plain dimensions of the name, and `3x3Refinery` counts as three by three. [Base adjacency](/systems/base-adjacency/) covers how far that footprint reaches for placement, and [`Bib=yes`](/keys/bib/) covers why it does not grow when a structure is bibbed.

## The irregular sizes

`3x3Refinery` is three cells by three but stands on eight of the nine, leaving the cell two east and one south of its top-left corner open. Its exit ring is a single cell, its own top-left corner, so anything it produces is put down inside it.

`0x0` stands on no cells at all, has no exit ring, and reports a width and height of zero, so a structure given it marks nothing on the map and has nowhere to put a finished object.

`6x4` is the one plain name whose block is not a rectangle: it stands on 21 cells, its bottom row being three cells wide and starting one east. Its exit ring is a single cell, two east and one north of its top-left corner. The shared Z shape is the depth mask a structure writes as it draws; it is what decides whether an object standing next to the structure is drawn in front of it or behind it. A structure six or more cells wide is drawn without it, so [`ZShapePointMove`](/keys/zshapepointmove/) has nothing to move.

## The second reading

The size is read twice, and the two readings are not equivalent. The first is from the structure's `[<Image ID>]` art entry; the second is from the art entry named after the BuildingType itself, and it replaces the first only when it resolves to something other than `1x1`.

:::caution[The type's own entry cannot shrink a footprint to one cell]
Because the override is skipped whenever the second reading comes out as `1x1`, writing `Foundation=1x1` in the type's own art entry leaves a larger size taken from the image entry standing. An unrecognized name written there is skipped for the same reason, where the same name in the image entry would have reset the footprint to one cell.
:::
