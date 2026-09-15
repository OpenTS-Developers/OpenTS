---
key: BurstDelay3
summary: The gap in frames between the fourth and fifth shot of a burst.
see_also: [Burst, ROF, BurstDelay0, BurstDelay1, BurstDelay2]
when_omitted:
  kind: value
  value: "-1"
---

```ini title="rules.ini"
[MyChaingun] ; example WeaponType
Burst=5
BurstDelay3=3 ; three frames between the fourth shot and the fifth
```

The value is the reload delay the firing object is given once its fourth shot of a burst has gone off, in game frames. `-1` asks for a random three to five frames instead. The assignment is reached only on a weapon whose [`Burst`](/keys/burst/) is above `4`; at a burst of exactly four the fourth shot is the last one and pays [`ROF`](/keys/rof/).

This is the last gap the weapon can name. The fifth shot and every one after it takes a random three to five frames whatever these assignments say, because the table holds only four entries and no assignment reaches past it.
