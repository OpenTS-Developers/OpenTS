---
key: VeteranSight
summary: Sight range of an object holding the sight ability is multiplied by one more than this value.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

The multiplier reaches only an object whose rank grants it the `SIGHT` ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/). Aircraft never read the value; their sight reveal reads the type's own range alone. The default doubles the radius, and `0` leaves it unchanged. Every floating-point value has an effect, because the multiplier is one more than the value: `0.5` widens the radius to one and a half times, while `-1` leaves nothing to reveal.

Scaling is applied to the type's sight range after the height-based increase that lifts an object standing on high ground. The widened radius is used the next time the object reveals terrain, not at the moment it is promoted.
