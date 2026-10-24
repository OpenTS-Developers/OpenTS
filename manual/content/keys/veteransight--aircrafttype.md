---
key: VeteranSight
scope: aircrafttype
label: Veteran sight range of a type
see_also: [Sight, EliteSight, "system:map-visibility", "system:veterancy"]
when_omitted:
  kind: value
  value: "-1"
  note: The veteran uses the type's `Sight=` value.
---

`VeteranSight` sets the sight range, in cells, of a type's veteran objects. It replaces [`Sight=`](/keys/sight/) for veterans of the type; rookies and below-rookie objects keep `Sight=`. The key is read from the type's section in `rules.ini` for aircraft, structures, infantry and vehicles.

Only a positive value takes effect. `0` and negative values leave the veteran on `Sight=`; `VeteranSight=0` changes nothing.

An elite whose type leaves [`EliteSight`](/keys/elitesight/) unset or non-positive uses this value.

```ini title="rules.ini"
[MYTANK] ; example UnitType
Sight=6
VeteranSight=8
EliteSight=10
```

The type's rookies use 6 cells, its veterans 8, and its elites 10. [Sight range](/systems/map-visibility/#sight-range) adds the height bonus and the `SIGHT` ability multiplier to that figure, and caps every look at ten cells.
