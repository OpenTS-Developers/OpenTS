---
title: Keep a scenario's free radar through low power
category: feature
release: 0.2.0
breaking: true
migration:
- Move `FreeRadarOnLowPower=` from `[General]` to `[Basic]` in each map file that sets it.
- For a value set in `[General]` of `rules.ini`, add it to `[Basic]` of each map that needs it; a value there no longer applies.
- Set `FreeRadar=yes` in `[Basic]` of each of those maps; without it the key does nothing.
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
