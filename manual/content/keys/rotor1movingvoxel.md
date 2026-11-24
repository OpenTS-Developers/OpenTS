---
key: Rotor1MovingVoxel
summary: VXL model used for rotor 1 above ground level.
when_omitted:
  kind: value
  value: ""
---

`Rotor1MovingVoxel` names rotor 1's VXL model for an aircraft above ground level, including while hovering. The engine adds `.VXL` and reads the file only when [`VoxelRotors=yes`](/keys/voxelrotors/) is set. [Aircraft rotors](/systems/aircraft-rotors/#voxel-artwork) explains model selection and fallback.
