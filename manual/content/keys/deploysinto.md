---
key: DeploysInto
summary: The BuildingType a vehicle turns into when it is given the deploy order.
see_also: ["UndeploysInto", "BaseUnit"]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[MCV]
DeploysInto=GACNST ; GDI Construction Yard
```

Naming a type gives the vehicle the deploy cursor when it is the only object selected and the pointer is over it. A vehicle whose weapon repairs friendly units keeps that cursor too.

The deploy order first checks that the structure can be placed. If it cannot, a player's vehicle plays the "cannot deploy here" speech and drops the order, and a computer house's vehicle tells its own and allied units standing on the site to move, so a later attempt can succeed. If it can, the vehicle turns to the structure's deploy facing, then disappears as the structure appears and plays its buildup.

Where the structure lands depends on the structure, not on the vehicle. One that is really a deployed vehicle (one flagged [`SensorArray=`](/keys/sensorarray/), [`TickTank=`](/keys/ticktank/), [`ICBMLauncher=`](/keys/icbmlauncher/), [`Artillary=`](/keys/artillary/), [`IsMobileStealth=`](/keys/ismobilestealth/), [`IsJuggernaut=`](/keys/isjuggernaut/), [`IsCoreDefender=`](/keys/iscoredefender/) or [`IsLimpetMine=`](/keys/islimpetmine/)) is placed on the vehicle's own cell. Every other structure is placed one cell to the north-west. The new structure inherits the house it belongs to, the side its artwork follows, its selection group, its crew's experience and any limpet drone clamped to it. Anything that was shooting at the vehicle transfers its target to the structure, except a [`VehicleThief=`](/keys/vehiclethief/) infantryman that was after the vehicle itself: against a construction yard or an [`IsMobileWar=yes`](/keys/ismobilewar/) structure it gives up the shot.

Three other paths read the value on a UnitType. A house's [`BuildLimit`](/keys/buildlimit/) count for the vehicle adds the number of structures of the named type the house already owns, so deploying an MCV does not free a slot to build another. A vehicle that deploys is treated as slow-witted for the purpose of a team that guards its slower members. And a human player's vehicle that deploys into an [`Artillary=yes`](/keys/artillary/) structure never retaliates on its own.

A name that matches no registered BuildingType registers a new, unconfigured structure under that name rather than failing. The values `none` and `<none>` resolve to nothing at all, and a rules file that writes one clears a value an earlier file set.

The key is accepted in an AircraftType, BuildingType, InfantryType or UnitType section, but only a vehicle ever deploys.
