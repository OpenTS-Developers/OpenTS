---
title: Ignore allied sensors around cloaked structures
category: fix
release: 0.1.0
targets:
- type: system
  id: cloaking
  effect: changed
- type: key
  id: Sensors
  effect: changed
credit: [ZivDero, Rampastring]
---

A structure now keeps or begins its cloak while a nearby detector's house treats the structure as an ally. Both structure checks take the alliance from the detector's house, as the check for moving vehicles, infantry and aircraft already did.
