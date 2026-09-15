---
key: VeinholeWarhead
summary: WarheadType vein damage is applied with.
see_also: ["system:veins", "VeinDamage", "Veinhole"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[CombatDamage]
VeinholeWarhead=VeinholeWH

[VeinholeWH] ; the WarheadType vein damage is applied with
Veinhole=yes
```

The warhead decides what [`VeinDamage`](/keys/veindamage/) costs each armor class, and [`Veinhole=yes`](/keys/veinhole/) on it is what lets a hurt object shoot back at the monster that grew the veins.

:::caution[Veins with no warhead are harmless]
This setting has no built-in value. Damage with no warhead is reduced to zero and dropped before any object's strength changes. A field with the setting unresolved therefore plays its attack over every cell it covers and takes nothing off anything standing there.
:::
