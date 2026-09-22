---
title: Hide player names in a quick match
category: feature
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
- type: system
  id: chat
  effect: changed
- type: system
  id: multiplayer-score-screen
  effect: changed
credit:
- ZivDero
- dkeeton
---

`QuickMatch=yes` in `[Settings]` of `spawn.ini` now shows the players as `Player 1` to `Player 8` wherever the match names them, with the same number for a player on every machine. The names stay in the launch file and the log.

A name longer than 39 characters no longer overruns the radar's player list.

dkeeton is credited for the ts-patches patch that first hid names in a quick match.
