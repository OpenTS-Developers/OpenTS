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

`SkipScoreScreen=yes` in a launch file ends a skirmish or network match without its score screen. The round is still counted and an ending movie the file asked for still plays. A map's own `SkipScore` keeps its campaign meaning and does not reach that screen.
