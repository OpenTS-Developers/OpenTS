---
title: Fix a crash when Tiberium debris lands on a slope
category: fix
release: 0.2.0
targets:
- type: key
  id: IsTiberium
  scope: voxelanimtype
  effect: changed
credit:
- eliasylanen
---

Voxel debris with `IsTiberium=yes` that seeded Tiberium on a sloped cell could crash the game. A meteor could crash the same way when it seeded a sloped cell around its impact. The sloped cell now gets one of the two slope overlays for its slope direction, the same choice Tiberium makes when it spreads onto a slope.
