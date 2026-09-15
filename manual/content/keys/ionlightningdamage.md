---
key: IonLightningDamage
summary: The raw damage each ion storm lightning bolt delivers.
see_also: [IonStormWarhead, "system:ion-storms"]
when_omitted:
  kind: value
  value: "500"
---

```ini title="rules.ini"
[General]
IonLightningDamage=500
```

The figure is spent twice over. It is [applied through `IonStormWarhead`](/systems/ion-storms/#what-a-strike-does) across the standard 1.5-cell explosion radius with no source recorded, so armor, distance, and the warhead's own table decide what each object actually loses and no house is credited with a kill. It also selects the explosion animation for the strike and, when the warhead sets [`Bright=yes`](/keys/bright/), sizes the spotlight flash.

The same value is used by the scripted [Lightning strike at...](/mapping/actions/taction-ion-lightning-strike/) action, which fires the identical routine at a waypoint whether or not a storm is running.
