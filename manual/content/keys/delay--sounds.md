---
key: Delay
scope: sounds
label: Silence between cycles
see_also: [Loop, Control]
when_omitted:
  kind: value
  value: "0"
---

One or two numbers: the silence between cycles of a looping sound, drawn at random between the two for every gap, or fixed when there is one. A number is in milliseconds, and a number with a decimal point is in seconds, so `250 750` and `0.25 0.75` mean the same thing. With `PREDELAY` in the sound's [`Control=`](/keys/control/) the silence comes once, before the first sample, instead of between cycles.

```ini title="sound01.ini"
[CRICKETS]
Control=LOOP RANDOM
Delay=2 6
```

A gap is measured on the game's own clock, so it is only as exact as a game frame, while cycles without a gap follow one another to the sample. A loop that a value above zero divides into cycles starts each cycle afresh, so its attack and decay still play only once, at the ends.
