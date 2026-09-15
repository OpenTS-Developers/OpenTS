---
title: Guard sends harvesters to work and holds other units where they stand
category: feature
release: 0.2.0
targets:
- type: command
  id: GuardObject
  effect: changed
credit: [ZivDero, AlexB, dkeeton, CCHyper]
---

The Guard command now orders a selected harvester or weeder to harvest, unless it is unloading at
the time; an unarmed harvester used to ignore the key. Every other selected object that can move
and fire guards the area around the spot it stands on when the key is pressed, rather than around
the cell it was heading for, so a unit on the move stops instead of finishing its trip first.

AlexB and CCHyper are credited for the harvester behavior in ts-patches and Vinifera, and
dkeeton for the ts-patches change that holds units in place.
