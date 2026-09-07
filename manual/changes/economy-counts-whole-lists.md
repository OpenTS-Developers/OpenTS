---
title: Count every listed refinery and harvester
category: fix
release: 0.2.0
targets:
- type: key
  id: HarvesterUnit
  effect: changed
- type: key
  id: BuildRefinery
  effect: changed
- type: key
  id: BuildWeapons
  effect: changed
credit: [ZivDero, AlexB]
---

The computer's money logic now counts every `BuildRefinery` and `HarvesterUnit` entry, and
prices and queues the first entry the country it acts as may own, falling back to entry 0. It
read entry 0 alone, so a computer house whose refinery or harvester was a later entry sold its
base to replace what it already had, and never queued a replacement harvester; the harvester
census that spreads a house's harvesters across a field counts them all too. An empty list no
longer crashes the game.

AlexB is credited for the ts-patches bundle that first read these lists whole.
