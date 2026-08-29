---
title: Seat a house at the start position it asked for
category: feature
release: 0.2.0
targets:
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

A house may now be placed at a named map start position rather than one the game picks. A
position is named by the map's own waypoint number, so a map that declares some of its
first eight waypoints and not others keeps the numbering it wrote: an undeclared waypoint
stays a gap instead of shifting the positions after it. A position the map does not declare,
or one another house has already taken, falls back to the game's own choice.

A game that names no positions is placed exactly as before, drawing the same random numbers
in the same order.

The house record in a saved game gained the start position the house was placed at. Saved
games from other versions were already refused; within this unreleased development cycle,
saves made before this change do not interoperate with builds made after it.
