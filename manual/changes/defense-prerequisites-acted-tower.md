---
title: Test defense prerequisites against the acted side's towers
category: fix
release: 0.2.0
targets:
- type: key
  id: WallTower
  effect: changed
- type: key
  id: AIWallTowers
  effect: changed
- type: system
  id: ai-base-building
  effect: changed
credit: [ZivDero]
---

A computer house choosing a base defense now excludes the wall towers its acted side lists from
the buildings it tests candidate prerequisites against, not the single type `WallTower` names.
A house whose side omits that type dropped it from the basis without putting it back, so a
defense naming it as a prerequisite was passed over even while the house owned one.
