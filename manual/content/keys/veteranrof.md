---
key: VeteranROF
summary: Reload delay of an object holding the rate of fire ability is divided by this value plus one.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: "1"
---

This figure is a divisor, so raising it shortens the wait between shots: the default halves the reload delay, and `0` leaves it unchanged. It reaches only an object whose rank grants it the `ROF` ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/).

Several paths settle the delay before this step, so it is never shortened for any of:

- a sonic weapon;
- a weapon whose spark, fire, or railgun particle system is attached;
- a building holding more than one round of ammunition;
- the shots inside a burst.

Only the delay that follows the final shot of a burst is reduced.
