---
key: BurstDelay1
summary: The gap in frames between the second and third shot of a burst.
see_also: [Burst, ROF, BurstDelay0, BurstDelay2, BurstDelay3]
when_omitted:
  kind: value
  value: "-1"
---

```ini title="rules.ini"
[MyTripleCannon] ; example WeaponType
Burst=3
BurstDelay1=6 ; six frames between the second shot and the third
```

The value is the reload delay the firing object is given once its second shot of a burst has gone off, in game frames. `-1` asks for a random three to five frames instead. The assignment is reached only on a weapon whose [`Burst`](/keys/burst/) is above `2`; at a burst of exactly two the second shot is the last one and pays [`ROF`](/keys/rof/).

The gap is taken as written, without the house rate-of-fire bias, the random padding or the veteran rate-of-fire ability that the burst's closing [`ROF`](/keys/rof/) is subject to. [`Burst`](/keys/burst/) covers how the shots of a burst are counted and which weapons ignore the short gap altogether.
