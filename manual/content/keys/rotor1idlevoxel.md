---
key: Rotor1IdleVoxel
summary: VXL model used for rotor 1 while idle.
when_omitted:
  kind: value
  value: ""
---

`Rotor1IdleVoxel` names rotor 1's VXL model for an aircraft while idle. The engine adds `.VXL` and reads the file only when [`VoxelRotors=yes`](/keys/voxelrotors/) is set. [Aircraft rotors](/systems/aircraft-rotors/#voxel-artwork) explains model selection and fallback.
