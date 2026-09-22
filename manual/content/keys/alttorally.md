---
key: AltToRally
summary: Makes the force-move key set a factory's rally point and the plain click move the structure.
see_also: [Factory, UndeploysInto, "system:production"]
when_omitted:
  kind: value
  value: "no"
---

A structure whose [`Factory=`](/keys/factory/) is `UnitType`, `InfantryType` or `AircraftType` has a [rally point](/systems/production/#rally-points). By default, a plain click on the ground sets the rally point and the force-move key gives the structure a move order. On a factory that can [undeploy](/keys/undeploysinto/), such as a mobile war factory, the move order packs it up and the vehicle drives to the clicked point. Any other factory does nothing on the force-move key.

`AltToRally=yes` swaps the two keys: the force-move key sets the rally point and the plain click gives the move order. A factory that cannot undeploy then does nothing on a plain click.

The setting does not affect a structure without a rally point. One that can undeploy, such as a construction yard or a deployed artillery piece, takes the move order on a plain click either way.

The game reads the setting only at startup. It affects only the player who sets it, so players in one match can use different settings.
