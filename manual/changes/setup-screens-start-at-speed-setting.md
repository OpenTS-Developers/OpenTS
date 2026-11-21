---
title: Start the skirmish and multiplayer setup screens at the speed setting
category: fix
release: 0.3.0
targets:
- type: key
  id: GameSpeed
  effect: changed
credit:
- ZivDero
---

The skirmish setup screen and the host's multiplayer setup screen used to start at the speed of the last game set up in either, or at the fastest speed before the first one. They now start at the player's `GameSpeed` setting.
