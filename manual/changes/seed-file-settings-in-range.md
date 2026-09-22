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

A seed file, or a map file with `RandomMap=yes`, now has each `[RandomMap]` setting pulled into the range the random map dialog allows, and a setting it leaves out takes its default. An out-of-range value such as `NumPlayers=9` used to read past the generator's tables, a `RegionSize` below `-10` hung it, and a missing setting kept whatever the generator last held, so two machines could build different maps.

`Seed=-1` now becomes `0` on this path, so a seed file shipped with it builds the map that seed `0` gives.
