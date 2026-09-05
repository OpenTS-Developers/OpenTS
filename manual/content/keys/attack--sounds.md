---
key: Attack
scope: sounds
label: Attack sample count
see_also: [Decay, Sounds, Control]
when_omitted:
  kind: context-dependent
  note: "`1` when `Control=` carries `ATTACK`, else `0`."
---

How many of the first names in [`Sounds=`](/keys/sounds/) are attack samples. A play draws one of them at random and plays it once before the body; a loop plays it only before its first cycle. Writing the key sets the count outright, and `ATTACK` in [`Control=`](/keys/control/) without it means one.

```ini title="sound01.ini"
[ENGINE]
Sounds=START1 START2 RUN STOP
Control=LOOP ATTACK DECAY
Attack=2
```

When the attack and decay counts together leave no body sample, every sample is treated as body and the counts are ignored.
