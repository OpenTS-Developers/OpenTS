---
key: Secondary
summary: The WeaponType in the object type's second weapon slot.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: none
---

Weapon choice scores both slots against each target and takes the higher, so the second slot is used only where it beats the first. Each slot scores as its warhead's [`Verses`](/keys/verses/) percentage against the target's armor multiplied by `1000`, doubled while the target is inside that slot's range and zeroed while the slot cannot fire; a tie goes to [`Primary`](/keys/primary/). The elite substitution never touches this slot.

```ini title="rules.ini"
[MYTANK] ; example UnitType
Primary=MyCannon      ; each names its own weapon section
Secondary=MyMachineGun
```

Writing `none` or `<none>` empties the slot, and an ID with no section of its own registers a WeaponType with only its defaults.

:::danger[An empty second slot can still be chosen]
When the first slot's warhead is [`Webby=yes`](/keys/webby/), weapon choice returns the second slot for every target the web weapon could fire at but cannot web. That covers every vehicle and building, a landed aircraft, and every immobilized or [`IsWebImmune=yes`](/keys/iswebimmune/) infantry. Scoring tolerates an empty slot. The retaliation test does not: it reads the warhead out of the second slot without checking that the slot is filled, through a weapon pointer that is not there. An object with a web weapon and no second weapon therefore crashes the moment it tries to retaliate against such a target. Always give a web weapon a companion in this slot.
:::
