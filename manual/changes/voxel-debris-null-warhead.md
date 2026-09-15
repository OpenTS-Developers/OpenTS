---
title: Skip voxel debris bounce damage without a warhead
category: fix
release: 0.1.0
targets:
- type: key
  id: Warhead
  scope: voxelanimtype
  effect: changed
credit:
- ZivDero
- AlexB
---

A voxel-debris bounce whose `rules.ini` section names no `Warhead=` now skips only its damage pass. The piece continues through its bounce effects and physics without passing a null warhead into each cell occupant's damage path or replacing the debris type's configured `Damage` with zero.
