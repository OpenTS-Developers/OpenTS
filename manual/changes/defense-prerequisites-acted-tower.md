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

A computer house choosing a base defense now excludes every wall tower its acted side lists in `AIWallTowers` from the buildings it tests candidate prerequisites against. `AIWallTowers` is read from the acted side's own section of rules.ini. Previously the test excluded only the single type `WallTower` names, in `[General]`. A house whose side omits that type dropped it from the basis without putting it back, so a defense naming it as a prerequisite was passed over even while the house owned one.
