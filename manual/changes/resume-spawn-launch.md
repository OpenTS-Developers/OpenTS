---
title: Resume a saved game from a client's launch file
category: feature
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit: [ZivDero]
---

A launch file with `LoadSaveGame=yes` now resumes the saved game it names. The save carries
the kind of game it was, the options it was played under and the houses that played it, so
nothing else in the file is consulted — which is what clients already write, their resume
files naming little beyond the save.

A save the folder does not hold, one made by another version of the game, and one made in a
game against other machines each refuse the launch with the reason shown; resuming a game
against other machines arrives with the network work it needs.

Which saved game a launch file names is now part of the identity two machines compare a
match by, since a resume is a match of its own.
