---
key: DebrisTypes
summary: The voxel wreckage a destroyed object throws, spent in list order against its debris budget.
see_also: [MaxDebris, DebrisMaximums, MetallicDebris, ExplosiveVoxelDebris]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
MaxDebris=6
DebrisTypes=MYSCRAP,MYTIRE ; VoxelAnimTypes registered in [VoxelAnims]
DebrisMaximums=4,2
```

The list is only reached when [`MaxDebris`](/keys/maxdebris/) is above zero, and it replaces the generic fallback rather than adding to it: a type with a budget and no list of its own throws [`MetallicDebris`](/keys/metallicdebris/) animations instead.

Entries are spent strictly in order. Each entry contributes a count drawn at random from zero up to its own [`DebrisMaximums`](/keys/debrismaximums/) figure, trimmed to whatever is left of the budget, and the pieces are created at the object's center owned by the object's house. The walk stops as soon as the budget runs out or the list ends, so a later entry is reached only when the earlier ones drew low counts. An entry after several generous ones may never appear at all.

Writing `DebrisTypes=none` empties the list and sends the type back to the fallback. Writing the key with nothing after the `=` is a different thing: the read finds no value and keeps whatever an earlier rules file set.
