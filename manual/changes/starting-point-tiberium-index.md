---
title: Plant each player's starting tiberium on their own start point
category: fix
release: 0.2.0
targets:
- type: key
  id: TiberiumLayout
  effect: changed
credit: [ZivDero]
---

The extra field a generated map plants at a player's start point sits on that player's own waypoint again. OpenTS 0.1.0 shifted every one by a player, so the first player got none and the last grew from a waypoint no start point had been given. In a debug build that tripped an assertion. In a release build the field grew from cell 0.
