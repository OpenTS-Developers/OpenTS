---
key: VeteranSpeed
summary: Movement speed of an object holding the faster ability is multiplied by one more than this value.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

The multiplier reaches only an object whose rank grants it the `FASTER` ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/). The default doubles speed, and `0` leaves it unchanged.

Scaling is applied after the house ground-speed bias and the object's own speed bias, and before the current throttle setting. Only infantry, vehicles, and aircraft have a speed to raise; a building that reached veteran rank gains nothing from the ability.
