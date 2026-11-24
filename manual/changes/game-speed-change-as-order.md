---
title: Leave the speed in SUN.INI alone during a multiplayer game
category: fix
release: 0.3.0
targets:
- type: key
  id: GameSpeed
  effect: changed
credit:
- ZivDero
---

A multiplayer game used to change the player's `GameSpeed` setting to the speed chosen in its setup screen, and then to each speed any player picked during the game, so the next save of the settings wrote that speed to `sun.ini`. A multiplayer game now leaves the setting alone, including a speed the player picks during it.

In a campaign mission or skirmish, a speed picked in the game controls dialog now takes effect after the next frame instead of before it.
