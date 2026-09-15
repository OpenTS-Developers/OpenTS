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

The game now saves on its own at a fixed interval of frames. A campaign rotates through `AUTOSAVE1.SAV` to `AUTOSAVE5.SAV`, and a skirmish through `AUTOSAVE_SKIRMISH1.SAV` to `AUTOSAVE_SKIRMISH5.SAV`. Against other machines, a client-launched game writes a numbered multiplayer save on every machine at the same frame. The interval for a game started from the menu comes from the new `AutoSaveInterval` setting, 10800 frames unless changed. A client-launched game takes its interval from its launch file's `AutoSaveGame`, which the launch file reader previously read without acting on. The launch file's `NextSPAutoSaveId` and `NextSkirmishAutoSaveId` now seed that rotation, and every save records which slot comes next, so a loaded game picks it up.
