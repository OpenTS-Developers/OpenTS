---
key: DebrisMaximums
summary: The largest number of pieces each debris type may contribute, one figure per entry.
see_also: [DebrisTypes, MaxDebris, MetallicDebris]
when_omitted:
  kind: value
  value: ""
---

```ini title="rules.ini"
[MYTANK] ; a UnitType registered in [VehicleTypes]
MaxDebris=6
DebrisTypes=MYSCRAP,MYTIRE ; VoxelAnimTypes registered in [VoxelAnims]
DebrisMaximums=4,2 ; at most four MYSCRAP, then at most two MYTIRE
```

The list is positional against [`DebrisTypes`](/keys/debristypes/): the first figure caps the first debris type, the second caps the second, and so on. Each figure is an upper bound on a random draw, not a fixed count: the entry contributes anywhere from zero pieces up to that many, and the draw is trimmed again by whatever remains of the [`MaxDebris`](/keys/maxdebris/) budget. A figure of `0` therefore never produces a piece from its debris type and hands the whole remaining budget to the next entry.

The list is read only while debris types are being walked, so it does nothing on a type that declares no [`DebrisTypes`](/keys/debristypes/).

:::danger[Every debris type needs a figure]
The figure is fetched by the debris type's own position with no check that this list is long enough. A type that names debris types and no maximums at all reads through a null pointer and crashes. One that names fewer maximums than debris types reads whatever follows the list in memory for the surplus entries, and a `-1` found there divides by zero.
:::
