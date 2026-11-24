---
key: Rotor3MovingVoxel
summary: VXL model used for rotor 3 above ground level.
when_omitted:
  kind: value
  value: ""
---

`Rotor3MovingVoxel` names rotor 3's VXL model for an aircraft above ground level, including while hovering. The engine adds `.VXL` and reads the file only when [`VoxelRotors=yes`](/keys/voxelrotors/) is set. [Aircraft rotors](/systems/aircraft-rotors/#voxel-artwork) explains model selection and fallback.
