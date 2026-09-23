---
title: Leave observers the whole map on Reshroud
category: fix
release: 0.2.0
targets:
- type: action
  id: TACTION_RESHROUD
  effect: changed
- type: mission
  id: TMISSION_RESHROUD
  effect: changed
- type: system
  id: observers
  effect: changed
credit: [ZivDero]
---

Reshroud map and its team mission no longer shroud an observer or a player defeated outside coach mode. Before, both covered the map for the player at every machine, so an observer saw only shroud until the map was revealed again.
