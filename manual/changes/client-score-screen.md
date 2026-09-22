---
title: Pass over the score screen when a launch file asks
category: feature
release: 0.2.0
targets:
- type: format
  id: spawn-ini
  effect: changed
- type: system
  id: multiplayer-score-screen
  effect: added
credit:
- ZivDero
- Rampastring
- CCHyper
---

`SkipScoreScreen=yes` in `[Settings]` of `SPAWN.INI` ends a skirmish or network match without its score screen. An ending movie that `PlayMoviesInMultiplayer` asked for still plays. A map's own `SkipScore` keeps its campaign meaning and does not reach that screen.
