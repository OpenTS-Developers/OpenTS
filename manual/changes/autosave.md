---
title: Save the game automatically at a fixed interval
category: feature
release: 0.2.0
targets:
- type: key
  id: AutoSaveInterval
  effect: added
- type: format
  id: save-games
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
- Rampastring
---

The game now saves on its own at a fixed interval of frames. A campaign rotates through
`AUTOSAVE1.SAV` to `AUTOSAVE5.SAV` and a skirmish through `AUTOSAVE_SKIRMISH1.SAV` to
`AUTOSAVE_SKIRMISH5.SAV`; a client-launched game against other machines writes a numbered
multiplayer save on every machine at the same frame. A game started from the menu takes its
interval from the new `AutoSaveInterval` setting, 10800 frames unless changed, and a
client-launched game from its launch file's `AutoSaveGame`, which the launch file reader
previously read without acting on. The launch file's next-slot keys now seed the rings, and
every save records where the rings stand so that a loaded game continues from them.

Saves made by earlier development snapshots of this cycle no longer load, because the
record of loose global values grew by the two ring positions.
