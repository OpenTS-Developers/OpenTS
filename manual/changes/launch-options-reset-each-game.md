---
title: Start each menu game without a loaded save's launch options
category: fix
release: 0.3.0
targets:
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
---

A game started from the menu used to keep the coach mode, building off allies, automatic MCV deployment, neutral targeting, scrap metal, shared control and `AutoSurrender` settings of a save loaded earlier in the same session. Only a launch file or a save sets these options, so a game started from the menu now starts with all of them off and gives the base of a player who leaves to the computer.
