---
title: Keep voxel debris damage local to each impact
category: fix
release: 0.1.0
targets:
- type: key
  id: Damage
  scope: voxelanimtype
  effect: changed
credit:
- ZivDero
- AlexB
---

Every object reached by a voxel-debris bounce now receives the `Damage=` the debris type states in `rules.ini` independently. Armor and distance still reduce each hit, but that reduction no longer overwrites the shared type value and weakens every later bounce and expiration blast from the same debris type.
