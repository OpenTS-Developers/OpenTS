---
key: Loop
scope: sounds
label: Loop count
see_also: [LoopLimit, Control, Delay]
when_omitted:
  kind: value
  value: "0"
---

With `LOOP` in the sound's [`Control=`](/keys/control/), how many times the body plays. The attack plays once before the first cycle and the decay once after the last. `0` plays the body until the game stops the sound, as when it scrolls out of range. The game fades such a loop out, and its decay does not play. Without `LOOP` the key has no effect.

```ini title="sound01.ini"
[ALARM]
Sounds=ALARMIN ALARM ALARMOUT
Control=LOOP ATTACK DECAY
Loop=4
```

[`LoopLimit=`](/keys/looplimit/) is read when this key is absent.
