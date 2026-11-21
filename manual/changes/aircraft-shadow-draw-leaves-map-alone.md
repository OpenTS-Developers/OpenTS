---
title: Draw an aircraft's shadow without moving the aircraft on the map
category: fix
release: 0.3.0
targets: []
credit:
- ZivDero
---

Drawing a voxel aircraft no longer changes the game. To draw the shadow, the engine set the aircraft's height to the ground and back, and each change took the aircraft off the map and put it back. On a machine that drew a landed aircraft, the aircraft became the first object in its cell; on a machine that drew a flying aircraft, the aircraft was briefly placed in the cell below it and could be revealed to the local player. A machine that did not have the aircraft on screen did none of this, so the machines in a network game could come to hold different games.
