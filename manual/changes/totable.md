---
title: Let the rules refuse a carryall a vehicle
category: feature
release: 0.2.0
targets:
- type: key
  id: Totable
  effect: added
- type: key
  id: Carryall
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, CCHyper]
---

A carryall lifted any allied vehicle it was pointed at. `Totable=no` now refuses the lift at the
cursor and in the mission alike, so neither a force-move nor a computer-controlled carryall can
take the vehicle either.
