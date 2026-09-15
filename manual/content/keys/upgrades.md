---
key: Upgrades
summary: How many upgrade plugs the structure accepts.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: "0"
---

A structure accepts plugs until the count is reached, and each plug it holds contributes its own [`Power=`](/keys/power/#scope-buildingtype), weapon and superweapon to the host. Reading this value also decides how many `PowerUp1Anim` through `PowerUp<n>Anim` art entries, with their damaged forms and offsets, are read for the type.

```ini title="rules.ini"
[MYPOWR]   ; example power plant that accepts one turbine
Upgrades=1

[MYTURBINE] ; example turbine plug
PowersUpBuilding=MYPOWR
Power=50
```

:::danger[Three plugs is the hard limit]
A runtime instance keeps room for three plugs. A type that advertises more lets a fourth be installed, and that fourth plug is stored one slot past the end of that store, over the record of the last superweapon the structure launched. A structure that afterwards launches a superweapon reads that record as a position in the declared list, without checking it, and the game crashes. Its art entries land in the [first active animation slot](/systems/building-animations/#the-upgrade-slots-and-the-active-slots-share-one-array) at the same time, overwriting whatever that slot was configured with. Keep the value at 3 or below.
:::
