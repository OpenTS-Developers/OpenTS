---
key: OmniHealer
summary: Lets a healing weapon mend infantry and vehicles alike.
see_also: ["Mechanic", "Damage", "AmbientDamage", "Passengers", "Verses", "system:repair", "system:target-selection", "system:warheads"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MEDIC] ; an InfantryType registered in [InfantryTypes]
OmniHealer=yes
```

Where [`Mechanic=yes`](/keys/mechanic/) exchanges one kind of patient for the other, this key adds one: the healer keeps infantry and gains the vehicles, landed aircraft and deployed buildings that [`Mechanic`](/keys/mechanic/) lists. Setting both is the same as setting this one alone.

Like `Mechanic`, it redirects a healer rather than creating one, and does nothing without a weapon whose [`Damage`](/keys/damage/#scope-weapontype) averages below zero.

The cursor follows the patient: a damaged allied soldier draws the heal cursor and a damaged allied vehicle draws the repair cursor, from the same object, and the [automatic scan](/systems/target-selection/) picks up both.

An infantry or a vehicle acts on the key. On a healing vehicle it is the only route to infantry.
