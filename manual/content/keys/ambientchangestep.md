---
key: AmbientChangeStep
summary: How far the ambient light moves on each step of a fade, as a fraction of full ambient light.
see_also: [AmbientChangeRate, Ambient, IonAmbient, "system:ion-storms"]
when_omitted:
  kind: value
  value: ".1"
---

```ini title="rules.ini"
[AudioVisual]
AmbientChangeStep=.1
```

The value is multiplied by 100 to give the step in the same hundredths [`Ambient`](/keys/ambient/) is stored in. `.1` therefore moves the level by ten hundredths each time. Steps are clamped at the target, so a fade never overshoots and the last step is whatever remains.

Every completed step raises the flag that admits the [Ambient light <= ...](/mapping/events/tevent-ambient-less-than/) and [Ambient light >= ...](/mapping/events/tevent-ambient-greater-than/) trigger events to that frame's evaluation. Those events are only ever tested while a fade is actually stepping.

The [Set ambient step...](/mapping/actions/taction-set-ambient-step/) trigger action overwrites this value in the loaded rules, and the overwrite outlives the trigger that made it.
