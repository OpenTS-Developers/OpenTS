---
key: VoxelBarrelFile
summary: The voxel model a building's gun barrel is drawn from.
see_also: ["BarrelAnimIsVoxel", "TurretAnim", "VoxelBarrelScale", "TurretAnimIsVoxel"]
when_omitted:
  kind: value
  value: ""
---

The value is a file name without its extension: `<name>.VXL` is loaded as the barrel, with the `<name>.HVA` beside it. At most 15 characters are kept.

```ini title="rules.ini"
[MYARTILLERY] ; a BuildingType registered in [BuildingTypes]
TurretAnim=MYART_A ; an AnimType registered in [Animations]; has no TUR
BarrelAnimIsVoxel=yes
VoxelBarrelFile=MYARTBAR ; the barrel is drawn from MYARTBAR.VXL
```

Only a [`BarrelAnimIsVoxel=yes`](/keys/barrelanimisvoxel/) building whose turret name yields no barrel name of its own reads the key. A [`TurretAnim`](/keys/turretanim/) name yields one when a scan finds `TUR` in it from the fifth character onward, compared case-blind. The name is cut at the first match, that `TUR` becomes `BARL`, and everything after it is dropped. A `TUR` that sits only inside the first four characters is never seen, so a building with such a name reads this key instead. This setting is then ignored, as it is on a building that draws a voxel turret rather than a voxel barrel.
