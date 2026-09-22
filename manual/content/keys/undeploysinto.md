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

Selling the structure or ordering it to pack up runs its deconstruction. When that animation finishes, the structure becomes one vehicle of the named type instead of credits and survivors; destroying it does not. It is placed on the structure's own cell when the structure is one of the deployed-vehicle kinds and one cell to the south-east otherwise, facing the direction that kind deploys at. It carries over the structure's health as a proportion of its own full strength, floored at one point. It also takes over the side its artwork follows, its selection group, its experience, any limpet drone clamped to it, its attached tag, its selection, and any move order the structure was holding. Everything that was shooting at the structure retargets onto the vehicle, except an engineer trying to capture it, which loses its target. Only if the vehicle cannot be placed at all is the money refunded instead.

The "structure sold" announcement is suppressed and no crew leaves the building. A structure holding no destination from a move order also becomes ready to finish its deconstruction partway through the animation rather than only at its last frame.

Not every structure that names a type may actually undeploy. The undeploy path also requires the structure to be one of the deployed-vehicle kinds, or a mobile war factory or limpet mine. Outside a campaign game, a structure holding a destination from a move order also passes, as long as it belongs to a human house in a session set up with the MCV redeploy option on. A [`ConstructionYard=yes`](/keys/constructionyard/) structure refuses in a campaign game, refuses for a computer house anywhere, and refuses in any session where redeploying is switched off.

Naming a type also makes the structure count as a vehicle rather than a building. It takes the move cursor, it can be given a destination, and it answers a movement test against the target cell. A band selection picks it up the way it picks up a unit instead of requiring a click of its own. A construction yard is deliberately excluded from the selection and vehicle-counting parts of that, so it keeps behaving like a structure, and a band selection also skips an [`IsMobileWar=yes`](/keys/ismobilewar/) structure.

If the structure's [`Factory=`](/keys/factory/) is `UnitType`, `InfantryType` or `AircraftType`, a plain click on the ground sets its [rally point](/systems/production/#rally-points), and the move order needs the force-move key. [`AltToRally=yes`](/keys/alttorally/) swaps the two. Any other structure, including a construction yard, takes the move order on a plain click.

A name that matches no registered UnitType registers a new, unconfigured vehicle under that name rather than failing. The values `none` and `<none>` resolve to nothing at all, exactly as if the key had been left out.

:::caution[Only a structure's section is read]
The key is accepted in an AircraftType, BuildingType, InfantryType or UnitType section, but every reader runs on structures alone, so a value set on any other type is stored and never read.
:::
