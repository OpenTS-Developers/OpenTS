---
title: Keep a scenario's free radar through low power
category: feature
release: 0.3.0
breaking: true
targets:
- type: key
  id: FreeRadarOnLowPower
  effect: added
- type: key
  id: FreeRadar
  effect: changed
- type: system
  id: power
  effect: changed
- type: format
  id: save-games
  effect: changed
credit:
- JoyfulShush
---

`FreeRadarOnLowPower=yes` in `[Basic]` of a map file keeps the local player's radar map up while the house is short of power, as long as the same file sets `FreeRadar=yes`. An ion storm still takes the map down, and without `FreeRadar=yes` the key does nothing.
