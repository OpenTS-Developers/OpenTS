---
key: LowRadarBrightness
scope: theater
label: Theater radar floor
see_also: [HighRadarBrightness]
when_omitted:
  kind: context-dependent
  note: "`0.8` for SNOW, which keeps its original settings; `1.0` for TEMPERATE and for every other theater."
---

A cell draws on the radar in two colors taken from its tile artwork, and this scales both of them at ground level. Below `1.0` the theater reads darker than its artwork, which is how snow keeps its pale ground from washing the radar out.

```ini title="rules.ini"
[DESERT]
LowRadarBrightness=1.0
```

A cell above ground level is interpolated between this and [`HighRadarBrightness`](/keys/highradarbrightness/) by its height, so the two together set how much relief the radar shows.
