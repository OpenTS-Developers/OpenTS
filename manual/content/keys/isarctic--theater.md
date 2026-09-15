---
key: IsArctic
scope: theater
label: Arctic theater
see_also: [IsIceGrowthEnabled, TemperateOccupationBits, SnowOccupationBits]
when_omitted:
  kind: context-dependent
  note: "`yes` for SNOW, which keeps its original settings; `no` for TEMPERATE and for every other theater."
---

A terrain type has two sets of occupation bits, [`TemperateOccupationBits`](/keys/temperateoccupationbits/) and [`SnowOccupationBits`](/keys/snowoccupationbits/), because artwork drawn for snow can cover different ground than the same object drawn for temperate. This flag says which of the pair a theater uses, and so which sub-positions of a cell a tree or a rock blocks. The pair is fixed, so a theater picks one side of it rather than declaring a third.

```ini title="rules.ini"
[DESERT]
IsArctic=no     ; uses TemperateOccupationBits
```

It also darkens the waypoint path line, which would be lost against pale ground. In the [random map generator](/keys/biome/) it suppresses veinholes and dims the ambient light.
