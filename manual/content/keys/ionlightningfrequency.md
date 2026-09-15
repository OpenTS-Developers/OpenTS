---
key: IonLightningFrequency
summary: Ten times this value is the chance in 1001 that an ion storm calls down a lightning bolt on a frame.
see_also: [IonLightningRandomness, "system:ion-storms"]
when_omitted:
  kind: value
  value: "25"
---

The value is multiplied by ten as it is read and truncated to a whole number, so `IonLightningFrequency=25.5` stores `255`. A storm calls a bolt on a frame when a draw from `0` through `1000` inclusive falls below the product. At `25` that is 250 of 1001 outcomes, a bolt on roughly one frame in four.

```ini title="rules.ini"
[General]
IonLightningFrequency=25
```

A frame that calls a bolt does not always produce one: [where it strikes](/systems/ion-storms/#where-it-strikes) can end with an empty candidate list, and that frame passes without a strike.
