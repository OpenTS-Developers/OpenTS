---
key: VeinholeTypeClass
summary: TerrainType every veinhole monster in the scenario reports as its own.
see_also: ["system:veins", "IsVeinhole", "VeinholeMonsters"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[General]
VeinholeTypeClass=VEINTREE

[VEINTREE]
Name=Veinhole Tree
Image=None
Armor=None
Strength=1000
IsVeinhole=true
```

One setting covers every [veinhole monster](/systems/veins/#veinhole-monsters) on the map. A monster has no type of its own: it reports this type whenever the engine asks what it is. The section's [`Strength`](/keys/strength/) is therefore the monster's maximum and current strength, and its [`Armor`](/keys/armor/) and [`Immune`](/keys/immune/) decide what damage does to it. [`IsVeinhole=yes`](/keys/isveinhole/) on the same section is what makes the monster clickable and a legal target.

:::danger[An unresolved setting faults while the map loads]
This setting has no built-in value, and a monster reads its maximum strength off the type without checking what it got. A map holding a veinhole overlay with the setting unresolved faults as the monster is created.
:::

:::danger[A named section that does not exist yields an unkillable monster]
A value naming a section the rules do not contain still produces a TerrainType. It keeps the negative maximum strength it was constructed with, because the fallback to [`TreeStrength`](/keys/treestrength/) only runs when the section is present. Every monster then spawns below zero strength, and damage is refused before its armor is read, so nothing can kill it. The read order itself is not the problem: `[General]` is read before the object sections, so a type first named here still reads its own section whenever one exists.
:::
