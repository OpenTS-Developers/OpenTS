---
key: Ambient
summary: The scenario's overall light level, where 1 is full daylight.
see_also: [IonAmbient, AmbientChangeRate, AmbientChangeStep, "system:ion-storms"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="map file"
[Lighting]
Ambient=1
```

The value is stored in hundredths, so `.5` is half daylight and `1.35` is brighter than daylight. It is applied at load without a fade. When [an ion storm ends](/systems/ion-storms/#the-storm-ends) the map returns to this level, and the return travels through the ambient fade rather than snapping.

The engine reads this value before its ion counterpart in the same section, and [`IonAmbient`](/keys/ionambient/) falls back to it. A map that gives only this key therefore darkens no further during a storm.

The [Set ambient light...](/mapping/actions/taction-set-ambient-light/) trigger action replaces the stored level during play. While a storm is running the replacement is stored but the fade is withheld until the storm clears.
