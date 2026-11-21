---
title: Start a game from a launch request
category: feature
release: 0.3.0
targets:
- type: command
  id: launch:launch
  effect: added
- type: format
  id: launch-request
  effect: added
credit:
- ZivDero
---

A launcher can start a campaign mission, a skirmish, a game against other machines, a saved game, or a map capture by writing a [launch request](/formats/launch-request/) file and passing it with `-LAUNCH=<file>`. When the request gives a result file, the game writes there whether it ran the request, why it refused it, and the outcome of each scenario it started.

The `Session identity:` line of an [out-of-sync report](/using/out-of-sync-reports/) is now a checksum of the launched game's description, for a game started from SPAWN.INI or from a launch request. It differs from the value earlier versions gave for the same SPAWN.INI, and a saved game resumed from SPAWN.INI no longer has one.
