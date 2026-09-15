---
key: HighRadarBrightness
scope: theater
label: Theater radar ceiling
see_also: [LowRadarBrightness]
when_omitted:
  kind: context-dependent
  note: "`1.1` for SNOW, which keeps its original settings; `1.6` for TEMPERATE and for every other theater."
---

This is the scale a cell's radar colors reach at the top of the height range. A cell is interpolated between [`LowRadarBrightness`](/keys/lowradarbrightness/) and this one by its own height. The gap between the two is how strongly high ground stands out from low.

```ini title="rules.ini"
[DESERT]
HighRadarBrightness=1.6
```

Setting it equal to `LowRadarBrightness` flattens the radar, and setting it below makes high ground read darker than low.
