---
key: VeteranSight
scope: global-rules
label: Rules-wide sight ability multiplier
see_also: [Sight, "system:map-visibility", "system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

Under `[General]` in `rules.ini`, `VeteranSight` multiplies the sight range of an object whose rank grants the `SIGHT` ability. Raising the value widens the area the object reveals: the default doubles its range, and `0` leaves it unchanged. The ability comes from [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/), as [abilities](/systems/veterancy/#abilities) describes.

Fractional values work: `0.5` gives one and a half times the range. At `-1` the multiplier is zero, so the object's range becomes zero. The result is a whole number of cells, with any fraction dropped.

The multiplier applies after the sight range for the object's rank and the height bonus that [`LeptonsPerSightIncrease`](/keys/leptonspersightincrease/) sets. An aircraft uses it too when its type grants the ability. [Sight range](/systems/map-visibility/#sight-range) gives the full calculation.

A promotion makes an object on the map look again, so its new range takes effect at once.
