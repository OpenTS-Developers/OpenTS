---
key: BlowupSound
summary: Sound a structure makes as damage takes it past a condition threshold.
see_also: [CrumbleSound, ConditionRed, Strength]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
BlowupSound=EXPLOSML ; a sound ID registered in SOUND.INI
```

The sound is played from the structure's own position, so it fades with distance from the view. It plays only on the hit that crosses a threshold: from at or above half the structure's maximum strength, rounded down, to below that, or from above [`ConditionRed`](/keys/conditionred/) of that strength to below it. Continued damage at the same condition is silent, and a hit that crosses both thresholds at once plays the sound once.

Only structures reach it. A vehicle, an infantryman or an aircraft crossing the same thresholds makes no sound of its own.
