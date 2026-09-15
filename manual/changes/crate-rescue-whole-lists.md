---
title: Judge the crate harvester rescue on the whole list
category: fix
release: 0.2.0
targets:
- type: system
  id: crates
  effect: changed
credit: [ZivDero, AlexB]
---

A unit crate now judges its free harvester on every `BuildRefinery` and `HarvesterUnit` entry. It hands out the first harvester the collector's country may own, where it used to compare entry 0 of each list alone. The random vehicle draw that follows now stops when no UnitType qualifies. A rules set where nothing is both `CrateGoodie=yes` and ownable by the collector used to hang the game on the spot.

AlexB is credited for the ts-patches bundle that first read these lists whole.
