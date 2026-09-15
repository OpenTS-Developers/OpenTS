---
key: Delay
scope: sounds
label: Silence between cycles
see_also: [Loop, Control]
when_omitted:
  kind: value
  value: "0"
---

One or two numbers: the silence between cycles of a looping sound. Every gap draws its length at random between the two, or is fixed when the key holds one number. A number is in milliseconds, and a number with a decimal point is in seconds, so `250 750` and `0.25 0.75` mean the same thing. With `PREDELAY` in the sound's [`Control=`](/keys/control/) the silence comes once, before the first sample, as well as between cycles.

```ini title="sound01.ini"
[CRICKETS]
Control=LOOP RANDOM
Delay=500 1500
```

A gap is timed in real milliseconds, and the engine checks it once per game frame, so it is only as exact as a frame. Cycles without a gap follow one another to the sample. A loop that a value above zero divides into cycles starts each cycle afresh. Its attack and decay still play only once, at the ends.
