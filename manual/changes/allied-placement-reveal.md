---
title: Reveal an ally's structure the moment it is placed
category: fix
release: 0.2.0
targets:
- type: system
  id: map-visibility
  effect: changed
- type: key
  id: AllyReveal
  effect: changed
credit: [ZivDero, Rampastring]
---

Outside a campaign, an ally's structure now lifts the player's shroud around itself as soon as it lands, as it already did on the ally's own screen. Only an object of the player's own house lifted the shroud when the player discovered it; now every discovered object does. A non-allied object still reveals nothing, and an ally's object reveals ground only while `AllyReveal=yes` holds. A campaign keeps the reveal to the player's own objects.
