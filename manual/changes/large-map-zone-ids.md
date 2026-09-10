---
title: Carry zone and subzone ids on large maps
category: fix
release: 0.2.0
targets:
- type: system
  id: route-search
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero]
---

Terrain zone and subzone identifiers are held as full integers rather than 16-bit values, so a map large enough to produce more than 32767 subzones no longer indexes the route search's tables with a negative number. A terrain change near the bottom or right edge of the playfield also no longer clears subzone identifiers past the end of the zone tables, and a route longer than 2000 cells is now abandoned instead of overrunning the move list it is written into.

The map record of a save game grew with those identifiers. A save written by an earlier development snapshot of this release cycle is refused when that record is read.
