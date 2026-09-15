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
- A weapon that fires as a beam uses the figure exactly as written, with none of the adjustments below. A beam weapon is one with [`IsSonic=yes`](/keys/issonic/), which is a property of the weapon itself, or a fire, spark or railgun weapon while its particle system is currently attached to the firer.
- A shot fired while the burst counter is still short of [`Burst`](/keys/burst/) takes its delay from the matching [`BurstDelay0`](/keys/burstdelay0/) to [`BurstDelay3`](/keys/burstdelay3/) entry instead. The delay after the first shot comes from `BurstDelay0`, the delay after the second from `BurstDelay1`, and so on. Where that entry is left unset, the delay is a random three to five frames.
- The delay after a burst's last shot, like every shot of a weapon without a burst, is the figure multiplied by the firing house's country [`ROF`](/keys/rof/#scope-housetype) multiplier, plus a random zero to two frames. A firer that has earned the rate-of-fire ability then divides the result by one plus [`VeteranROF`](/keys/veteranrof/).

Shot by shot, a `Burst=3` weapon with every `BurstDelay` entry set fires, waits [`BurstDelay0`](/keys/burstdelay0/), fires, waits [`BurstDelay1`](/keys/burstdelay1/), fires, then waits the adjusted figure before the burst starts over. `BurstDelay2` and `BurstDelay3` stay unused until [`Burst`](/keys/burst/) is raised past 3.

An aircraft's mission handlers wait the primary weapon's figure between passes, unadjusted, so this also sets how often an attacking aircraft reconsiders what it is doing.

:::danger[A weapon with no reload delay stops the game]
Three routines that rate an object's worth as an anti-air, anti-armor or anti-infantry threat divide by this figure in whole numbers, and none of them checks it first. They run whenever a base's defense zones are scored, whenever a computer house decides where to send an object, and whenever one hunts for a target in the field. The zones are scored for every house each time one of its buildings is placed, captured or sold. A weapon with `ROF=0`, or omitting the key, which stores the same value, therefore divides by zero and stops the game. The anti-air routine is reached only when the projectile is [`AA=yes`](/keys/aa/), the other two when it is [`AG=yes`](/keys/ag/).
:::
