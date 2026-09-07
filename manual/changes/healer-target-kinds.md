---
title: Let the rules aim a healing weapon at vehicles
category: feature
release: 0.2.0
targets:
- type: key
  id: Mechanic
  effect: added
- type: key
  id: OmniHealer
  effect: added
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, CCHyper, Rampastring]
---

A healing weapon mended whatever kind its owner was: a soldier only infantry, a vehicle only vehicles. `Mechanic=yes` now points a soldier's healing weapon at vehicles instead, and `OmniHealer=yes` points a healer of either kind at infantry and vehicles alike, cursor and automatic targeting together.

A healing vehicle dropped a landed aircraft or a deployed vehicle as a target the moment it could not fire, though it was allowed to mend both. It now holds one until the job is done.
