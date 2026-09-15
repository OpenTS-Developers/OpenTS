---
title: Show the written briefing when a mission has no briefing movie
category: feature
release: 0.2.0
targets:
- type: key
  id: Brief
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
credit:
- ZivDero
- Rampastring
- CCHyper
- dkeeton
---

A campaign mission whose `Brief` names no movie, or names one whose file is missing, now shows its written briefing as it starts. The page is the one the objectives button brings up during play. Only a fresh start shows it; a restart and the replay a loss offers go straight to the map. The mission's transit theme plays behind the page.
