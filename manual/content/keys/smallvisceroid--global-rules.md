---
key: SmallVisceroid
scope: global-rules
label: Spawned visceroid type
see_also: ["TiberiumDeathToVisceroid", "LargeVisceroid", "system:tiberium"]
when_omitted:
  kind: value
  value: none
---

The named UnitType is the creature Tiberium leaves behind. It is created for the Neutral house, not for whoever owned the victim. Two deaths reach it, and both need the scenario to declare [`TiberiumDeathToVisceroid=yes`](/keys/tiberiumdeathtovisceroid/): an infantryman killed by the Tiberium under its feet, and any object killed by a damaging particle. The new creature appears on the cell the death happened in. Nothing appears on a cell that a vehicle occupies or is crossing.

[Damage](/systems/tiberium/#damage) covers the hit that produces the first of those deaths.

```ini title="rules.ini"
[General]
SmallVisceroid=VISC_SML  ; the UnitType the shipped rules name
```

Naming a type here does not make it behave like a visceroid. Merging, wandering and the creature artwork all come from [`SmallVisceroid=yes`](/keys/smallvisceroid/#scope-unittype) in that type's own section, so a type named here without it is an ordinary vehicle.

The name here does not have to belong to a declared UnitType. A name that matches nothing does not resolve to no type. A UnitType of that name is brought into existence instead. It takes its statistics from a section of the same name, and stays blank if the rules never supply one. The literals `none` and `<none>` are the exception: those do resolve to no type.
