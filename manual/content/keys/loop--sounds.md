---
key: Loop
scope: sounds
label: Loop count
see_also: [LoopLimit, Control, Delay]
when_omitted:
  kind: value
  value: "0"
---

With `LOOP` in the sound's [`Control=`](/keys/control/), how many times the body plays; `0` plays it until the game ends the sound, as it does when a looping sound's source is destroyed or scrolls out of range. The attack plays once before the first cycle and the decay once after the last. Without `LOOP` the key has no effect.

```ini title="sound01.ini"
[ALARM]
Sounds=ALARMIN ALARM ALARMOUT
Control=LOOP ATTACK DECAY
Loop=4
```

[`LoopLimit=`](/keys/looplimit/) is read when this key is absent.
