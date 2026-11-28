---
key: AltToRally
summary: Makes the force-move key set a structure's rally point and the plain click move the structure.
see_also: [UndeploysInto, "system:production"]
when_omitted:
  kind: value
  value: "no"
---

At `no`, a plain click on the ground sets a structure's [rally point](/systems/production/#rally-points), and a click with the force-move key held gives it a move order. On a structure that can [undeploy](/keys/undeploysinto/), such as a mobile war factory, the move order packs it up and the vehicle drives to the clicked point. A structure that cannot move does nothing on a force-move click.

`AltToRally=yes` swaps the two clicks: a force-move click sets the rally point, and a plain click gives the move order. A structure that cannot move then does nothing on a plain click.

The setting does not affect a structure without a rally point. If such a structure can undeploy, as a deployed artillery piece can, a plain click gives it the move order at either setting.

The game reads the setting only at startup. It affects only the player who sets it, so players in one match can use different settings.
