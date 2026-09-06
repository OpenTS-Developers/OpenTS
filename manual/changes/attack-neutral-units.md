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

`AttackNeutralUnits=Yes` lets a target scan consider a neutral house; an absent key leaves
every such object passed over.

Separately, and in every game including campaigns, the rule keeping a player's units from
picking on buildings that cannot shoot back covers a building whose weapon has no range as
well as one carrying no weapon. The two together acquire the armed part of a neutral base and
leave the scenery.

dkeeton is credited for the ts-patches patch this follows and AlexB for the rule covering a
building whose weapon has no range.
