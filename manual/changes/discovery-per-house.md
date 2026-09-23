---
title: Record discovery the same way on every machine
category: fix
release: 0.2.0
targets:
- type: system
  id: map-visibility
  effect: changed
- type: event
  id: TEVENT_DISCOVERED
  effect: changed
- type: event
  id: TEVENT_HOUSE_DISCOVERED
  effect: changed
credit: [ZivDero]
---

In a network game, Discovered by player and House Discovered... followed each machine's own player, so a trigger could run on one machine and not on another and the game could fall out of sync. Both now fire the same way on every machine: the first time a human player other than the owner discovers the object.
