---
title: Leave the speed in SUN.INI alone during a game started from SPAWN.INI
category: fix
release: 0.3.0
targets:
- type: key
  id: GameSpeed
  effect: changed
credit:
- ZivDero
---

A game started from `SPAWN.INI` used to write its launch speed to `sun.ini` whenever the settings were saved during that game, such as when the player confirmed the game controls dialog. The speed in `sun.ini` now stays as it was during such a game, unless the player picks another speed during a campaign mission or skirmish.
