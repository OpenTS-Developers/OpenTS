---
title: Let a structure type refuse to claim map walls
category: feature
release: 0.3.0
targets:
- type: key
  id: WallOwner
  scope: buildingtype
  effect: added
- type: system
  id: walls-and-gates
  effect: changed
- type: format
  id: save-games
  effect: changed
credit:
- Shush
---

`WallOwner=no` in a structure type's `rules.ini` section stops that type's buildings from claiming the walls placed with the map, even when their owner's country allows it. The other structure types of that country still claim walls.
