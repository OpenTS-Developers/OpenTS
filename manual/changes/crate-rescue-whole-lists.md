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

A unit crate now judges its free harvester on every `BuildRefinery` and `HarvesterUnit` entry
and hands out the first harvester the collector's country may own, where it compared entry 0 of
each alone. The random vehicle draw that follows stops when no UnitType qualifies: a rules set
where nothing is both `CrateGoodie=yes` and ownable by the collector hung the game on the spot.
Where a type does qualify, the draw is unchanged.

AlexB is credited for the ts-patches bundle that first read these lists whole.
