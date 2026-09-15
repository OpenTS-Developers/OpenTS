---
key: Theater
scope: scenarios
label: Scenario theater
see_also: [IceGrowthEnabled]
when_omitted:
  kind: value
  value: "TEMPERATE"
---

```ini title="map file"
[Map]
Theater=SNOW
```

The theater is settled before any of the map's contents are read. It decides which tile, art and palette archives are mounted for the whole load. The name matches one of the declared theaters, ignoring case. The declared list comes from the rules file's [`[Theaters]`](/formats/rules-registries/) section; a setup whose rules declare no such section still has `TEMPERATE` and `SNOW`, in that order. The theater also decides whether the ice simulation runs, so [`IceGrowthEnabled`](/keys/icegrowthenabled/) does nothing in a theater whose [`IsIceGrowthEnabled`](/keys/isicegrowthenabled/) is off.

Scenario-wide settings live mostly in the map file's `[Basic]` section. This one goes under `[Map]`.

A name no theater answers to is logged, and the map is played in the first declared theater. The same fallback applies when the key is missing altogether.
