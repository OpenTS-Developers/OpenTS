---
key: DamageDelay
summary: Minutes between the damage ticks a house takes while it is short of power.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "1"
---

Every house has a timer that is reloaded with this many game minutes each time it expires, whether or not the house was short of power at that moment. The [structure damage tick](/systems/power/#the-structure-damage-tick) runs only when the timer expires and the house cannot meet its drain, so the value sets the cadence, not the condition. A house is first charged the delay when it is created.

```ini title="rules.ini"
[General]
DamageDelay=0.25   ; one check every 15 seconds
```
