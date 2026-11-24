---
title: Reset rule settings a map changed before the next game
category: fix
release: 0.3.0
targets:
- type: format
  id: multiplayer-rules
  effect: changed
- type: format
  id: powerups
  effect: changed
credit:
- ZivDero
---

A value that a map or `MPLAYER.INI` set in a section such as `[General]`, `[CombatDamage]`, `[Powerups]` or a land type section stayed in force in later games until the game was restarted, wherever the rules files left the key out. These sections now return to the values they had before the first scenario each time a scenario loads.
