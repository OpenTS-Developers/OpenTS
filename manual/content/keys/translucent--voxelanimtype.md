---
key: Translucent
scope: voxelanimtype
label: Voxel debris translucency
see_also: ["VoxelIndex", "IsTiberium"]
when_omitted:
  kind: value
  value: "no"
---

The model is blended into the scene at 50% for the whole of the piece's life. The level is fixed and does not vary as the piece ages. The shadow is drawn in a separate pass before it and is unaffected, so a translucent piece still casts a solid shadow.
