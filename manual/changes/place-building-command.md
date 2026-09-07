---
title: Add a Place Building hotkey command
category: feature
release: 0.1.0
targets:
- type: command
  id: ManualPlace
  effect: added
credit: [CCHyper, ZivDero]
---

A building that has finished construction and sits waiting on the sidebar can now be picked up from the keyboard. Placing one meant clicking its completed cameo, so a player who had dismissed the placement cursor had to travel back to the sidebar to recover it.

The command arrives unbound, under Interface in the keyboard options, so no existing keyboard file is disturbed. It does nothing while no building is complete, while the finished item is not a building, or while that building is already being placed, and it drops a superweapon cursor armed at the time so that placement begins rather than the superweapon reasserting its cursor.
