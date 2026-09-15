---
key: TiberiumSpreadRadius
summary: Selects which of the eight cells around a landing animation can take Tiberium.
see_also: ["IsTiberium", "TiberiumSpawnType", "system:tiberium"]
when_omitted:
  kind: value
  value: "0"
---

The setting is read only where the animation also declares [`IsTiberium=yes`](/keys/istiberium/#scope-animtype) and is thrown by [`Bouncer=yes`](/keys/bouncer/) or [`IsMeteor=yes`](/keys/ismeteor/#scope-animtype).

The growth is laid out by walking a square of offsets from minus the setting to plus it and discarding those that fall outside the radius. A surviving offset does not plant where its coordinates point. Its first coordinate names a compass direction: `0` is north, the count runs clockwise through northeast and east, and it is taken modulo `8`, so `-2` names west. The offset selects the neighbor of the impact cell in that direction, and the second coordinate only decides whether the offset survives the radius test. At setting `2`, the offsets `-2, 0` and `0, +2` both lie on the radius, but the first plants the cell due west of the impact and the second the cell due north. The reach is therefore never wider than the ring of eight cells touching the impact. The cell the animation came down on is never planted at all.

| Setting | Cells that can take Tiberium |
| --- | --- |
| `0` | north |
| `1` | northwest, north, northeast |
| `2` | west through east across the north |
| `3` | southwest through southeast across the north |
| `4` and above | all eight neighbors |

Raising the figure past `4` adds nothing but work. The same eight cells are examined more times each, and a cell that has already [germinated](/keys/istiberium/#scope-animtype) holds an overlay and is refused on every later visit.

:::caution[The default still plants]
`0` does not mean no spread. It keeps the single offset at the origin, and the origin selects the first compass direction. An animation flagged for Tiberium and left without this setting plants one cell to the north of where it landed. Every crystal and debris animation in the shipped `art.ini` that plants Tiberium either sets `0` or leaves the setting out.
:::
