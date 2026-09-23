---
key: AllyReveal
summary: Whether a house's objects also reveal terrain for its allies.
see_also: ["system:map-visibility"]
when_omitted:
  kind: value
  value: "yes"
---

At `yes`, an object's looks uncover the map for its owner's allies as well as for its owner, so an ally's sight shows on screen. At `no`, they uncover it for the owner alone. Outside a campaign every object also looks the moment the local player discovers it, so at `yes` a structure an ally places lifts the shroud around itself as soon as it goes down.

The flag also decides whether forming an alliance reveals anything at once. At `yes`, when a house makes another its ally, every object of the house that made the alliance looks, so the new ally sees what those objects see. Outside a campaign, the objects of a [passive house](/keys/multiplaypassive/) never look, so its alliances reveal nothing.

The [Enable Ally Reveal](/mapping/actions/taction-enable-ally-reveal/) and [Disable Ally Reveal](/mapping/actions/taction-disable-ally-reveal/) trigger actions overwrite this value in the loaded rules, and the overwrite outlives the trigger that made it.
