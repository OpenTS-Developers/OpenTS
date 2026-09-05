---
title: Recover from an out-of-sync game with a dialog and an in-game multiplayer load
category: feature
release: 0.2.0
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
before. The master of a client-launched match can also load a multiplayer save from the options
menu during play. The launch file's `IsHost` now names the master, which decides these things
and hands down the network timings; a file without it leaves the first seat in charge.

The dialog and the in-game load follow Vinifera's, by ZivDero and Rampastring.
