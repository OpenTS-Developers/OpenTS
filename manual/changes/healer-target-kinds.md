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

A healing weapon mended whatever kind its owner was: a soldier only infantry, a vehicle only vehicles. `Mechanic=yes` in a soldier type's own section in `rules.ini` now points its healing weapon at vehicles instead. `OmniHealer=yes` points a healer of either kind at infantry and vehicles alike, both for the cursor's heal action and for automatic targeting.

A healing vehicle dropped a landed aircraft or a deployed vehicle as a target the moment it could not fire, though it was allowed to mend both. It now holds one until the job is done.
