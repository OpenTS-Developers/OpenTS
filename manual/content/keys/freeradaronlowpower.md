---
key: FreeRadarOnLowPower
summary: Whether FreeRadar keeps the radar map up while the local player's house is short of power.
see_also: ["FreeRadar", "system:power"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
FreeRadar=yes
FreeRadarOnLowPower=yes
```

`FreeRadarOnLowPower=yes` keeps the radar map up while the local player's house is [short of power](/systems/power/), provided the same map also sets [`FreeRadar=yes`](/keys/freeradar/). 
An ion storm still takes the radar down.

Without `FreeRadar=yes`, the key does nothing: a player going into low power loses the radar. [Power output and drain](/systems/power/#radar) gives the full test.
