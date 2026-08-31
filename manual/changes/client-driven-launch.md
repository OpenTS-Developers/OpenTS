---
title: Launch and play a game from a client's launch file
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:spawn
  effect: added
- type: format
  id: spawn-ini
  effect: added
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, Rampastring, dkeeton, FunkyFr3sh, CCHyper, Belonit, hifi, Iran]
---

Starting the game with `-SPAWN` now plays the match `SPAWN.INI` describes: a skirmish, a
campaign mission, a game against other machines through a CnCNet tunnel or straight between
them, or any of those resumed from a saved game. The startup movies and the menu are
skipped, and the game exits when the match ends.

A client-launched game against other machines can now be saved from its options dialog. A
game set up from the menu is assembled as before.

The people credited here wrote the earlier spawners this one follows.
