---
title: Let a type set how many pips its row has
category: feature
release: 0.2.0
targets:
- type: key
  id: MaxPips
  effect: added
- type: key
  id: PipScale
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, Rampastring]
---

`MaxPips=` sets how many pips the row under a selected object holds. Without it each [`PipScale`](/keys/pipscale/) keeps its old length, and a structure's `Tiberium` or `Power` row still sizes itself from its footprint. An `Ammo` or `Passengers` row never holds more pips than the type holds ammunition or passengers.
