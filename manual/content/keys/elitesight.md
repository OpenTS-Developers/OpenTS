---
key: EliteSight
summary: Sight range for a type's elite objects.
see_also: [Sight, VeteranSight, "system:map-visibility", "system:veterancy"]
when_omitted:
  kind: value
  value: "-1"
  note: The elite uses the type's `VeteranSight` when that is positive, and otherwise its `Sight=` value.
---

`EliteSight` sets the sight range, in cells, of a type's elite objects. A positive value replaces both [`Sight=`](/keys/sight/) and the type's [`VeteranSight`](/keys/veteransight/#scope-aircrafttype). The key is read from the type's section in `rules.ini` for aircraft, structures, infantry and vehicles.

Only a positive value takes effect. `0` and negative values leave the elite on the type's `VeteranSight` when that is positive, and on `Sight=` otherwise; `EliteSight=0` changes nothing.

```ini title="rules.ini"
[MYINF] ; example InfantryType
Sight=4
VeteranSight=6
EliteSight=8
```

The type's rookies use 4 cells, its veterans 6, and its elites 8. A type that sets only `EliteSight` leaves its veterans on `Sight=`. [Sight range](/systems/map-visibility/#sight-range) adds the height bonus and the `SIGHT` ability multiplier to that figure, and caps every look at ten cells.
