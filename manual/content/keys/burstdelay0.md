---
key: BurstDelay0
summary: The gap in frames between the first and second shot of a burst.
see_also: [Burst, ROF, BurstDelay1, BurstDelay2, BurstDelay3]
when_omitted:
  kind: value
  value: "-1"
---

```ini title="rules.ini"
[MyTwinCannon] ; example WeaponType
Burst=2
BurstDelay0=4 ; four frames between the first shot and the second
```

The value is the reload delay the firing object is given once its first shot of a burst has gone off, in game frames. `-1` asks for a random three to five frames instead, and so does any burst whose [`Burst`](/keys/burst/) is not above `1`. The first shot is then also the last, and it pays [`ROF`](/keys/rof/).

The gap is taken as written, without the house rate-of-fire bias, the random padding or the veteran rate-of-fire ability that the burst's closing [`ROF`](/keys/rof/) is subject to. [`Burst`](/keys/burst/) covers how the shots of a burst are counted and which weapons ignore the short gap altogether.
