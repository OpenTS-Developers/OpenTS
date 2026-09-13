---
key: ROF
scope: weapontype
label: Weapon reload delay
see_also: [Burst, IsSonic, VeteranROF, Ammo]
when_omitted:
  kind: value
  value: "0"
---

The value is the game frames a firer waits before the shot after this one, at 15 frames to the second. It is a delay rather than a rate, so a larger figure fires more slowly.

```ini title="rules.ini"
[120mm]
Damage=70
ROF=80
```

Four things reshape it on the way to the firer.

- A structure holding more than one round of [`Ammo`](/keys/ammo/) waits a single frame instead, so the figure decides only the pause after its last round.
- A weapon that fires as a beam — [`IsSonic=yes`](/keys/issonic/), or one whose spark, fire or railgun particle system is currently attached to the firer — uses the figure exactly as written, with none of the adjustments below.
- A shot fired while the burst counter is still short of [`Burst`](/keys/burst/) takes its delay from the matching [`BurstDelay0`](/keys/burstdelay0/) to [`BurstDelay3`](/keys/burstdelay3/) entry instead, or from a random three to five frames where that entry is left unset. The first shot of a burst consults no entry at all, so it always takes the random three to five.
- Every other shot is the figure multiplied by the firing house's country [`ROF`](/keys/rof/#scope-housetype) multiplier, plus a random zero to two frames, and then divided by one plus [`VeteranROF`](/keys/veteranrof/) if the firer has earned the rate-of-fire ability.

An aircraft's mission handlers wait the primary weapon's figure between passes, unadjusted, so this also sets how often an attacking aircraft reconsiders what it is doing.

:::danger[A weapon with no reload delay stops the game]
Three routines that rate an object's worth as an anti-air, anti-armor or anti-infantry threat divide by this figure in whole numbers, and none of them checks it first. They are entered whenever a base's defense zones are scored — which happens for every house each time one of its buildings is placed, captured or sold — whenever a computer house decides where to send an object, and whenever one hunts for a target in the field. A weapon carrying `ROF=0`, or omitting the key, which stores the same value, therefore divides by zero and stops the game. The anti-air routine is reached only when the projectile is [`AA=yes`](/keys/aa/), the other two when it is [`AG=yes`](/keys/ag/).
:::
