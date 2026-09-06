---
title: Resolve spawn house owners in multiplayer scenarios
category: feature
release: 0.2.0
targets:
- type: format
  id: scenario-objects
  effect: changed
- type: system
  id: starting-forces
  effect: changed
- type: key
  id: House
  effect: changed
credit:
- ZivDero
- Iran
- Rampastring
---

Vehicle, infantry, aircraft, and structure rows, trigger definitions, and TeamTypes may now be owned by `Spawn1` through `Spawn8` or `<Player @ A>` through `<Player @ H>`, meaning whoever starts at that numbered position, and trigger and team mission house parameters accept the same positions as `50` through `57` or `4475` through `4482`. Start positions are settled as the scenario loads, before those records are read. A spawn house nobody holds resolves to nothing, so its rows are skipped and its triggers and teams are dropped.

Iran is credited for the CnCNet spawner houses this follows and Rampastring for the later
fixes to them.
