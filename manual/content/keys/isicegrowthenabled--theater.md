---
key: IsIceGrowthEnabled
scope: theater
label: Theater ice
see_also: [IsArctic, IceGrowthEnabled, IceGrowthRate]
when_omitted:
  kind: context-dependent
  note: "`yes` for SNOW, which keeps its original settings; `no` for TEMPERATE and for every other theater."
---

Ice is a snow feature the rest of the simulation is gated on. Where this is off, no ice sheet grows or is smoothed, a vehicle crossing ice never cracks it, and [`IceGrowthEnabled`](/keys/icegrowthenabled/) on a map does nothing.

```ini title="rules.ini"
[DESERT]
IsIceGrowthEnabled=no
```

It also decides one thing at load time. In a theater with ice, the land type of the ice edge tiles is rewritten to water, so that units treat the open edge of a sheet as the sea it floats in. A theater without ice keeps whatever land types its tiles declare.
