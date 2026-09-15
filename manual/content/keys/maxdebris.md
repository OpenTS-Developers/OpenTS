---
key: MaxDebris
summary: The number of pieces of wreckage a destroyed object may throw.
see_also: [DebrisTypes, DebrisMaximums, MetallicDebris, ExplosiveVoxelDebris]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
MaxDebris=6
DebrisTypes=MYSCRAP,MYTIRE ; VoxelAnimTypes registered in [VoxelAnims]
DebrisMaximums=4,2
```

A figure of zero turns wreckage off, and it is the first thing tested, so neither debris list is read at all. Above zero, the figure is a budget rather than a count, and which list it is spent on depends on whether the type declares [`DebrisTypes`](/keys/debristypes/) of its own:

- **With a list**, the budget is handed out along it in order, up to the matching [`DebrisMaximums`](/keys/debrismaximums/) figure from each entry and only as far as the budget stretches. Every piece is created at the object's center, owned by the object's house. A budget larger than the sum of those figures simply goes unspent; the walk stops at the end of the list.
- **Without a list**, a count is drawn at random from zero up to the budget and that many [`[AudioVisual] MetallicDebris`](/keys/metallicdebris/) animations are created, each picked independently, twenty leptons above the object's center.

The wreckage is thrown before any [`Explodes=yes`](/keys/explodes/#scope-aircrafttype) blast and after the death voice. An object that a collapsing bridge or a shove over water has dropped, and that then died within ten leptons of the water, leaves the death path before either and throws nothing.
