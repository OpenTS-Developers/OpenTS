---
title: Pick the nearest docking bay on a large map
category: fix
release: 0.2.0
targets:
- type: key
  id: Dock
  effect: changed
- type: system
  id: tiberium
  effect: changed
credit: [ZivDero]
---

A unit more than about 181 cells from its docking buildings now goes to the nearest one. The distance comparison used to overflow at that range, so a harvester on a large map drove to the farthest refinery and an aircraft to the farthest pad.
