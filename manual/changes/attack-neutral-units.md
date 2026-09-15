---
title: Auto-target neutrals on AttackNeutralUnits
category: feature
release: 0.2.0
targets:
- type: system
  id: target-selection
  effect: changed
- type: format
  id: spawn-ini
  effect: changed
breaking: false
credit:
- ZivDero
- dkeeton
- AlexB
---

Outside a campaign, `AttackNeutralUnits=Yes` lets a player's units target a neutral house's objects by themselves. Without the key, units pass over every object of a neutral house.

A separate change applies in every game, campaigns included. The rule that keeps a player's units from picking on buildings that cannot shoot back now covers a building whose weapon has no range as well as one with no weapon. Together the two let a unit target the armed buildings of a neutral base and leave the unarmed ones alone.

dkeeton is credited for the ts-patches patch this follows and AlexB for the rule covering a building whose weapon has no range.
