---
title: Recover from an out-of-sync game with a dialog and an in-game multiplayer load
category: feature
release: 0.2.0
breaking: true
migration:
- Tools that waited for `SAVEGAME.NET` to appear must list `SVGM_nnn.NET` instead; the CnCNet client already does.
targets:
- type: system
  id: out-of-sync-recovery
  effect: added
- type: format
  id: save-games
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
- type: system
  id: network-packet-validation
  effect: changed
credit:
- ZivDero
- Rampastring
---

An out-of-sync game used to show a two-button box whose Continue dropped every connection. It now opens a dialog: the master loads one of the match's saved games, plays on, or quits, while everyone else waits with a player list and a chat box, and the lowest remaining seat takes over if the master leaves. Continue drops only the players whose checksum disagreed with this machine's. The master can also load a multiplayer save from the options menu during play, and the launch file's new `IsHost` names which seat is master; without it the first seat is.

Multiplayer saves are numbered by the game, `SVGM_000.NET` upward, in every network game, and a new match drops the previous match's files. A client-launched match writes `spawnSG.ini` at its first save. `SAVEGAME.NET`, which the client watched for to do both, is no longer written.

Two dialog faults are fixed with this. A dialog larger than the 640 by 400 backdrop art showed uninitialized memory past the art's edge and now shows black. A windowed game that lost the focus stopped painting its dialogs, so one opening then, such as the frame-sync reconnect dialog, stayed blank.

The dialog and the in-game load follow Vinifera's, by ZivDero and Rampastring.
