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
---

A service depot, hospital or armory can now hold a rally point. Select the structure and click the ground, and the objects it releases head there instead of staying beside the building.

With `AltToRally=yes` under `[Options]` in `sun.ini`, the force-move key sets the rally point instead, as it does for a factory.
