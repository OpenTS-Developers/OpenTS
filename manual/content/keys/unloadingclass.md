---
key: UnloadingClass
summary: The vehicle type one harvester is drawn as while it stands at a dock unloading.
see_also: [UnloadingHarvester, Harvester, Weeder, Dock, UndeploysInto, "system:tiberium"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[HARV]
UnloadingClass=HORV ; a UnitType registered in [VehicleTypes]
```

Naming a type here overrides the rules-wide [`UnloadingHarvester`](/keys/unloadingharvester/), which owns the exchange and its consequences, so each harvester in a set can carry its own empty-hopper artwork. [Unloading](/systems/tiberium/#unloading) covers the docking that gets it there.

Only a [`Harvester=yes`](/keys/harvester/#scope-unittype) or [`Weeder=yes`](/keys/weeder/#scope-unittype) vehicle is drawn from it, and for a vein harvester it is the only route to the exchange at all: the rules-wide value has never reached one.

A name that matches no registered UnitType registers a new, unconfigured vehicle under that name rather than failing. The values `none` and `<none>` leave a Tiberium harvester on the rules-wide value rather than suppressing the exchange, exactly as leaving the key out does.

The key is accepted in an AircraftType, BuildingType, InfantryType or UnitType section, but only a harvesting vehicle is ever drawn from it.
