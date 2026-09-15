---
key: VoxelIndex
summary: Which section of the voxel model a voxel animation is drawn from.
see_also: ["ShareSource", "ShareBodyData", "Translucent"]
when_omitted:
  kind: value
  value: "0"
---

A voxel model holds one or more sections and a voxel animation draws exactly one of them. The index selects it by position, counting from zero, for the shadow and for the lit model alike. A piece borrowing another type's model through [`ShareSource`](/keys/sharesource/) indexes that model's sections, which is how one vehicle file supplies several distinct pieces of wreckage.

:::caution[The shading table is always built from the first section]
The lighting lookup a piece is shaded through is precalculated from section zero however this is set. A model whose sections do not all use the same normal set therefore shades every section but the first through the wrong table.
:::

:::danger[An index past the last section reads outside the model]
Nothing measures the index against the number of sections the model holds. An index beyond the last one, or a negative one, takes the piece's bounding box from whatever memory lies outside the section table. The piece is drawn at a size and position that come from unrelated data, and an address far enough outside the tables faults and stops the game.
:::
