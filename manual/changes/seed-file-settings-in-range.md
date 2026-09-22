---
title: Hold a seed file's settings in range when it is played
category: fix
release: 0.2.0
targets:
- type: format
  id: map-seed
  effect: changed
- type: system
  id: map-generation
  effect: changed
credit:
- ZivDero
---

A seed file played as a scenario is now held to the ranges the random-map dialog allows before a map is built from it. A setting written outside its range, such as `NumPlayers=9` or `Biome=7`, used to read past the generator's tables, and a `RegionSize` below `-10` kept the generator working forever. Each is now pulled back to the nearest legal value.

`Seed=-1` becomes `0` on this path, so a seed file shipped with it now builds the map seed `0` gives.
