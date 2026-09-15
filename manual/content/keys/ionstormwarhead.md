---
key: IonStormWarhead
summary: Selects the WarheadType every ion storm lightning bolt detonates with.
see_also: [IonLightningDamage, IonImmune, "system:ion-storms"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[General]
IonStormWarhead=MyStormWH ; a WarheadType registered in [Warheads]
```

The warhead's [`Verses`](/keys/verses/) table, its [`Bright`](/keys/bright/) flag, and its animation lists shape [what a bolt does](/systems/ion-storms/#what-a-strike-does) to the objects around the strike point.

:::caution[This assignment also decides who is immune]
Explosion damage skips a team member whose TeamType is [`IonImmune=yes`](/keys/ionimmune/) when the detonating warhead is the one named here. The two are matched as the same warhead, not by name. Giving an ordinary weapon that warhead makes the weapon skip those objects too, and pointing this key at a different warhead moves the exemption with it.
:::
