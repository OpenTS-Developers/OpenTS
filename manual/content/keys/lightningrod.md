---
key: LightningRod
summary: Raises the type's chance of being picked as the target of an ion storm lightning bolt.
see_also: [IonImmune, IonLightningRandomness, "system:ion-storms"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[GAPOWR]
LightningRod=yes
```

An object without the flag enters an aimed bolt's [candidate list](/systems/ion-storms/#where-it-strikes) with a 2% chance. With the flag, a building that is switched on enters with a 42% chance, and a vehicle or an infantryman whose locomotor still has power enters with 12%. A switched-off building, or a vehicle or infantryman whose locomotor has lost power, falls back to the ordinary 2%. Aircraft are excluded from the list whatever they have.

:::caution[A lightning rod cancels team immunity]
A team member whose TeamType is [`IonImmune=yes`](/keys/ionimmune/) is normally left out of the candidate list. Setting this flag puts it back in, so a rod-bearing member of an immune team is aimed at like any other object — although the blast still spares it.
:::
