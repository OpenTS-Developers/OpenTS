---
title: Carry the scenario file in the save game
category: feature
release: 0.2.0
targets:
- type: format
  id: save-games
  effect: changed
- type: system
  id: campaign-progression
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
- type: format
  id: opents-ini
  effect: changed
credit:
- ZivDero
---

A save carries the scenario file it was played from when the deployment's `OPENTS.INI` sets
`CarryScenarioFile=yes`, and a restart or the replay after a loss then reads the mission
from that copy rather than from disk. That lets a mission resumed through the CnCNet client
be restarted: the client replaces `spawnmap.ini` with a stub on resume, and the restart read
the stub and crashed. The key is off by default, since a large map adds half again to a save.

Saves from earlier development snapshots of this cycle no longer load, because the scenario
record grew by the file.
