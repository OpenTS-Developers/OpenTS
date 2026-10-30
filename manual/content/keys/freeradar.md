---
key: FreeRadar
summary: Whether the scenario grants the radar map without a radar structure.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
FreeRadar=yes
```

The player has the radar map without owning a working [`Radar=yes`](/keys/radar/) structure. The radar map goes off during an ion storm. It also goes off while the player's house is short of power, unless the map sets [`FreeRadarOnLowPower=yes`](/keys/freeradaronlowpower/).
