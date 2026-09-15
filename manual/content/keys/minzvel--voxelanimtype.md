---
key: MinZVel
scope: voxelanimtype
label: Voxel debris launch speed
see_also: ["MaxZVel", "MaxXYVel", "IsMeteor"]
when_omitted:
  kind: value
  value: "3.5"
---

Leptons per frame: 256 leptons to a cell, 15 frames to the second. Debris is thrown upward at this speed plus a whole number of leptons drawn from the span up to [`MaxZVel`](/keys/maxzvel/). This is the slowest launch the type can produce. If [`MaxZVel`](/keys/maxzvel/) stands less than a lepton above it, the pick adds nothing and every piece launches at exactly this speed.

A type whose section reads as below throws its debris upward at 3 to 8 leptons per frame.

```ini title="rules.ini"
[MYDEBRIS]     ; a VoxelAnimType, declared under [VoxelAnims]
MinZVel=3
MaxZVel=8
```

A meteor takes the figure as its vertical speed outright, with no pick and no reference to the maximum. The sign then decides the approach, as [`IsMeteor`](/keys/ismeteor/#scope-voxelanimtype) describes.
