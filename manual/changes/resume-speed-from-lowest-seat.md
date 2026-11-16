---
title: Resume a game against other machines at one speed on every machine
category: fix
release: 0.3.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
---

A game against other machines resumed from `SPAWN.INI` used to run each machine at the `GameSpeed` of its own file, so files that set different speeds could put the machines out of sync. The resumed game now starts at the speed the match was playing at when it was saved, and the player in the lowest seat then changes it for everyone to the speed that player's file sets.
