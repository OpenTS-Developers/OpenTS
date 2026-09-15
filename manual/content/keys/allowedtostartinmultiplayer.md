---
key: AllowedToStartInMultiplayer
summary: Lets a vehicle or infantry type appear among the units each house is handed at a multiplayer start.
see_also: [BaseUnit, Cost, TechLevel, Owner, "system:starting-forces"]
when_omitted:
  kind: value
  value: "yes"
---

```ini title="rules.ini"
[MYSUPERTANK] ; a UnitType registered in [VehicleTypes]
AllowedToStartInMultiplayer=no
```

Only a UnitType's and an InfantryType's value is read; a BuildingType or AircraftType stores it and is never asked. The pass that reads it runs once, as a multiplayer or skirmish scenario finishes loading, and never in a campaign.

Allowing a type does two things. Every allowed vehicle except the [`BaseUnit`](/keys/baseunit/), and every allowed infantry type, contributes its [`Cost`](/keys/cost/#scope-aircrafttype) to the one average price that sets every house's budget. The type also joins the shortlist of each house whose [`TechLevel`](/keys/techlevel/#scope-aircrafttype) reaches it and whose [`Owner`](/keys/owner/) admits it. Denying a type therefore both removes it from every shortlist and moves the average that decides how many objects everybody gets. [Starting forces](/systems/starting-forces/) owns the budget, the shortlist, and the order the budget is spent in.

:::caution[A house can end up short of its budget]
Denying every type at once gives every house a zero budget, so each is placed with its base unit alone. Denying every InfantryType while a vehicle stays allowed leaves each house with the vehicles it drew before two thirds of its budget was spent, since only infantry may be drawn after that. [Starting forces](/systems/starting-forces/#when-placement-fails) covers the other shortlists that come up empty.
:::
