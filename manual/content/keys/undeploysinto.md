---
key: UndeploysInto
summary: The UnitType a structure turns back into when it is taken down.
see_also: ["DeploysInto", "UnloadingClass", AltToRally]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[GACNST]
UndeploysInto=MCV ; Mobile Construction Vehicle
```

A structure that names a vehicle type can pack up into one vehicle of that type. Selling the structure or giving it a move order starts its deconstruction, and the vehicle appears when the deconstruction ends. A structure without a destination ends its deconstruction partway through the animation, before the last frame. Destroying the structure never produces the vehicle.

A structure whose type has no [construction artwork](/keys/buildup/#a-type-with-no-construction-artwork-cannot-be-sold) is never taken down, so a sale or a move order does nothing.

The structure undeploys only if any of these holds:

- It has one of these type flags: [`SensorArray=yes`](/keys/sensorarray/), [`TickTank=yes`](/keys/ticktank/), [`ICBMLauncher=yes`](/keys/icbmlauncher/), [`Artillary=yes`](/keys/artillary/), [`IsMobileStealth=yes`](/keys/ismobilestealth/), [`IsJuggernaut=yes`](/keys/isjuggernaut/), [`IsCoreDefender=yes`](/keys/iscoredefender/#scope-buildingtype) or [`IsLimpetMine=yes`](/keys/islimpetmine/).
- It is an [`IsMobileWar=yes`](/keys/ismobilewar/) mobile war factory.
- Outside a campaign, it belongs to a human player in a session with the MCV redeploy option on, and it has a destination. A move order gives it one, and so does a structure's [rally point](/systems/production/#rally-points).

A structure that meets none of these is sold for its refund. A move order therefore sells a structure outside the first two groups in a campaign, or in a session with the MCV redeploy option off. It also sells one other than a construction yard when the game finds no reachable cell near the clicked point and the structure has no earlier destination.

The vehicle appears on the structure's cell for a structure with one of those flags, and on the cell to the south-east for any other structure. It faces north for an artillery structure, east for a sensor array, tick tank or ICBM launcher, and south otherwise. If the vehicle cannot be placed, the house receives the structure's refund instead.

The vehicle's health is the structure's fraction of full health applied to the vehicle's maximum, rounded down and never below 1. The vehicle keeps the structure's selection group, experience, attached tag, any limpet drone attached to it, and the country it was built for. It is selected if the structure was. If the structure had a destination, the vehicle moves there.

Everything that was targeting the structure switches its target to the vehicle, except an engineer, which loses its target.

A structure that names a type never lets out a crew and never plays the "structure sold" announcement, whether it undeploys or is sold. Of these structures, only a construction yard plays the sell sound.

Except for a construction yard, a structure that names a type counts as a [deployed vehicle](/systems/capture/#an-engineer-over-a-structure). It counts toward its house's vehicles instead of its buildings, cannot be repaired, and does not start the [base-attacked response](/systems/base-attacked/). A [band selection](/systems/band-selection/) picks it up unless it is an `IsMobileWar=yes` structure.

Except while an EM pulse has stunned it, the player can give the structure a move order. The move cursor appears over an empty cell that the structure's type could be built on, and over any cell of the playable area while Shift is held. A [`ConstructionYard=yes`](/keys/constructionyard/) structure takes a move order only outside a campaign, for a human player, in a session with the MCV redeploy option on.

If the structure has a [rally point](/systems/production/#rally-points), a plain click on the ground sets it, and a move order needs the force-move key. [`AltToRally=yes`](/keys/alttorally/) swaps the two. Any other structure, including a construction yard, takes the move order on a plain click.

A name that matches no registered UnitType registers a new, unconfigured vehicle type under that name. The values `none` and `<none>` clear the key, so the structure names no vehicle.

:::caution[Only a structure's section is read]
The key is accepted in an AircraftType, BuildingType, InfantryType or UnitType section, but only structures use it. A value set on any other type has no effect.
:::
