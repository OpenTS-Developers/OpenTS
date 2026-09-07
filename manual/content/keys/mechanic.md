---
key: Mechanic
summary: Turns a healing weapon away from infantry and onto vehicles.
see_also: ["OmniHealer", "Damage", "AmbientDamage", "Passengers", "Verses", "system:repair", "system:target-selection", "system:warheads"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[MECHANIC] ; an InfantryType registered in [InfantryTypes]
Mechanic=yes
```

The key redirects a healer; it does not create one. A healer is an object whose weapon [`Damage`](/keys/damage/#scope-weapontype) averages below zero across the slots it carries, and on anything else this key does nothing. Set, the healer takes vehicles as its patients and gives up infantry; [`OmniHealer=yes`](/keys/omnihealer/) grants both instead.

Vehicle here covers driven vehicles, landed aircraft, and deployed [`UndeploysInto`](/keys/undeploysinto/) buildings other than a construction yard. An ordinary building, an airborne aircraft, and anything not allied to the healer are never patients.

The redirection reaches both the cursor and the [automatic scan](/systems/target-selection/): a mechanic offers the repair cursor over a damaged ally, walks to one it finds while guarding, and holds it until it is whole. Force-move withdraws the offer over a vehicle with [`Passengers`](/keys/passengers/) capacity, which is what keeps a damaged transport boardable.

Only an infantry acts on the key; a healing vehicle takes vehicles as its patients with or without it. Nothing about [what a healing shot does on arrival](/systems/warheads/#healing) changes.
