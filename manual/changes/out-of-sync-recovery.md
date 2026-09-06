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

When a network game goes out of sync the game now opens a dialog in place of the two-button
box. The master chooses whether every machine loads one of the match's saved games, plays on
without the players out of sync, or quits; everyone else waits, with a player list and a chat
box, and the lowest remaining seat takes over when the master leaves. Continue now drops only
the players whose checksum disagreed with this machine's, where it dropped every connection
before. The master can also load a multiplayer save from the options menu during play. The
launch file's `IsHost` now names the master, which decides these things and hands down the
network timings; a file without it leaves the first seat in charge.

Multiplayer saves are now numbered by the game itself, `SVGM_000.NET` upward, in every network
game. A new match drops a previous match's files, and a client-launched match writes
`spawnSG.ini` for the client at its first save; the client did both when `SAVEGAME.NET`
appeared, and that file is no longer written.

Two dialog drawing faults are fixed with this. A dialog larger than the 640 by 400 backdrop
art, which the new dialogs are, showed uninitialized memory past the art's edge; that area is
now black. A windowed game also stopped painting its dialogs whenever another window had the
focus, so a dialog that opened then, such as the frame-sync reconnect dialog, never appeared
and stayed blank after the focus came back. Windowed games now paint dialogs without the
focus, and every game repaints them when the focus returns.

The dialog and the in-game load follow Vinifera's, by ZivDero and Rampastring.
