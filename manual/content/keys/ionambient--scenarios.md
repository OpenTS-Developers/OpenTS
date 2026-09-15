---
key: IonAmbient
scope: scenarios
label: Scenario lighting
see_also: [Ambient, AmbientChangeRate, "system:ion-storms"]
when_omitted:
  kind: context-dependent
  note: The value this scenario's Ambient key sets, read before it in the same section. With neither key present that is 1, full daylight.
---

```ini title="map file"
[Lighting]
Ambient=1
IonAmbient=.5
```

A storm does not apply this level outright. It becomes the target of [the ambient fade](/systems/ion-storms/#the-ambient-ramp), which moves the map's light level toward it in steps and back to [`Ambient`](/keys/ambient/) when the storm ends. The palette tint changes at once, so a map with a dark ion level shows its ion colors well before it shows its ion darkness.
