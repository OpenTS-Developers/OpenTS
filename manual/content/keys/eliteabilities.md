---
key: EliteAbilities
summary: The abilities an object of this type gains at elite rank, on top of its veteran abilities.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: ""
---

A comma-separated list of ability tokens, matched without regard to letter case and parsed exactly like [`VeteranAbilities`](/keys/veteranabilities/), including the whitespace rule described there. The list is read only at elite rank: a veteran of the same type gets nothing from it, and an elite holds the union of both lists. [The ability table](/systems/veterancy/#abilities) lists the eighteen accepted tokens and what each one does.

```ini title="rules.ini"
[MYTANK] ; example UnitType
VeteranAbilities=FIREPOWER
EliteAbilities=ROF,SELF_HEAL
```

An elite `MYTANK` fires harder, reloads faster, and repairs itself; a veteran one only fires harder.
