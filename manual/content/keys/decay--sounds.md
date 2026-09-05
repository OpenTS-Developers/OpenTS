---
key: Decay
scope: sounds
label: Decay sample count
see_also: [Attack, Sounds, Control]
when_omitted:
  kind: context-dependent
  note: "`1` when `Control=` carries `DECAY`, else `0`."
---

How many of the last names in [`Sounds=`](/keys/sounds/) are decay samples. A play draws one of them at random and plays it once after the body; a loop plays it after its last cycle, or after the current sample fades when the game ends the loop early. Writing the key sets the count outright, and `DECAY` in [`Control=`](/keys/control/) without it means one.

```ini title="sound01.ini"
[ENGINE]
Sounds=START RUN STOP1 STOP2
Control=LOOP ATTACK DECAY
Decay=2
```

When the attack and decay counts together leave no body sample, every sample is treated as body and the counts are ignored.
