---
key: IonLightningRandomness
summary: The percentage of ion storm lightning bolts that fall on a random cell rather than on an object.
see_also: [IonLightningFrequency, IonImmune, LightningRod, "system:ion-storms"]
when_omitted:
  kind: value
  value: "75"
---

```ini title="rules.ini"
[General]
IonLightningRandomness=75
```

A random bolt draws a cell until it lands inside the playfield, so it can strike empty ground. It can also strike ground outside the playable area that no player will ever occupy. The remaining bolts go through [the aimed selection](/systems/ion-storms/#where-it-strikes), which weighs every object on the map and can end up striking nothing at all.

At `100` no bolt is ever aimed, so [`LightningRod`](/keys/lightningrod/) and [`IonImmune`](/keys/ionimmune/) stop influencing where lightning falls. At `0` a storm over a map with no eligible object produces no bolts at all.
