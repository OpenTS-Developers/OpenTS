---
key: RepairBay
summary: The BuildingType a vehicle or aircraft heads for when it is ordered to be repaired.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "none"
---

```ini title="rules.ini"
[General]
RepairBay=GADEPOT   ; GADEPOT stands in for whichever BuildingType the mod uses

[GADEPOT]
UnitRepair=yes      ; the type named above needs this or it turns arrivals away
```

The match is on the exact type. A repair order looks for the nearest building of this type under **All of:**

- it belongs to the ordering house or an ally;
- a ground unit can reach it within the same movement zone;
- it is willing to take the object in.

A computer house's damaged aircraft use the same search. No repair order names any other destination, so the player reaches a second depot type only by driving a vehicle onto it. A damaged computer vehicle runs a search of its own, and that search covers every [`UnitRepair=yes`](/keys/unitrepair/) building regardless of this setting.

Whether the building can repair anything at all is [`UnitRepair`](/keys/unitrepair/)'s business. A type named here without that flag turns every arrival away, and a unit ordered to repair keeps searching for as long as its house owns one of them.

This setting does not decide whether a selected player-controlled aircraft can enter a clicked building. That direct action accepts any idle, empty `UnitRepair=yes` building or helipad.

:::danger[Leaving this unset crashes the game]
The value starts as no type at all, and both the search and the ownership test behind it read through it without checking. The first unit to take a repair order ends the game, including a computer harvester that has run out of Tiberium and heads for the pad on its own to be sold.
:::
