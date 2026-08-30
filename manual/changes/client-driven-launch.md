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

Starting the game with `-SPAWN` now plays the match `SPAWN.INI` describes: a skirmish
against seated computer players, a campaign mission, a game against other machines carried
over a CnCNet tunnel or straight between them, or any of those resumed from a saved game.
The startup movies, the main menu and the map list the menu would scan are all skipped, the
settings file a client manages is not written back to, and the game exits when the match
ends. The launch file's own page lists what the game takes from a file and what it does not.

A house can now be seated at a start position the file names, by the map's own waypoint
number; a game naming none is placed exactly as before, drawing the same random numbers in
the same order. The house record in a saved game gained that position, and the options
dialog of a game against other machines gained the Save Game button its synchronized save
always lacked.

The campaign handicap pair became session state, so a client may combine all nine pairings
where the menu offers its three coupled ones, and a restart or the next mission keeps the
pair. A game set up from the menu is assembled exactly as before.

The launch file's vocabulary is not this project's. It was settled by the CnCNet client and
by the spawners written for it before this one, and the game reads it as they wrote it; the
people who built those are credited above alongside this reading of it.
