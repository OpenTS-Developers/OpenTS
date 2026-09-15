---
title: Cancel placement and input modes when input locks
category: fix
release: 0.1.0
targets:
- type: action
  id: TACTION_LOCK_INPUT
  effect: changed
credit: [ZivDero, tomsons26]
---

Taking control away from the player now cancels a structure waiting to be placed. The completed structure stays in the factory and can be placed again. It also drops the repair, sell, power and waypoint modes before the lock takes hold, and unlocking does not restore them. Locking input used to leave all of them standing. A scripted sequence that began while the player held a building on the cursor therefore gave the mission back with the placement grid or a special cursor still active.
