---
title: Allow a Service Depot, a Hospital and an Armory to hold a rally point.
category: feature
release: 0.3.0
targets:
- type: system
  id: production
  effect: changed
- type: key
  id: UnitRepair
  effect: changed
- type: key
  id: Hospital
  effect: changed
- type: key
  id: Armory
  effect: changed
credit:
- JoyfulShush
- Rampastring
---

A service depot, hospital or armory can now hold a rally point. Select the structure and click the ground, and the objects it releases head there instead of staying beside the building.

A service depot takes a rally point in any movement zone. A released vehicle follows the point only when the point is in the same movement zone as the vehicle; otherwise it moves to an exit cell beside the depot. A released aircraft follows the point in any zone. 

A unit that cannot move into shroud ignores the rally point and moves to an exit cell near the depot.

With `AltToRally=yes` under `[Options]` in `sun.ini`, the force-move key sets the rally point instead, as it does for a factory.

Credits to Rampastring for the original Vinifera implementation.
