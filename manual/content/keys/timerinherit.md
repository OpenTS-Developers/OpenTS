---
key: TimerInherit
summary: Whether this mission resumes the countdown timer the previous one finished with.
see_also: [CarryOverMoney]
when_omitted:
  kind: value
  value: "no"
---

```ini title="map file"
[Basic]
TimerInherit=yes
```

The mission countdown is captured when a campaign mission is won and handed to the next one along with the [carried-over money](/keys/carryovermoney/). The timer handoff works independently of the money settings. With the switch set, and only if the captured timer had time left on it, the new mission starts that timer running from where the last one stopped. With the switch clear, or with a captured timer of zero, the new mission starts with its countdown stopped and a trigger action has to start one.

Losing and replaying captures nothing new: the timer stored when the previous mission was won is applied again each time the mission restarts.
