---
title: Set a rally point with a plain click
category: feature
release: 0.2.0
breaking: true
migration:
- Set `AltToRally=yes` under `[Options]` in `sun.ini` to keep the old controls, where the force-move key set the rally point and the plain click moved a deployed factory.
targets:
- type: key
  id: AltToRally
  effect: added
- type: key
  id: IsMobileWar
  effect: changed
- type: system
  id: production
  effect: changed
credit: [ZivDero, AlexB]
---

A plain click on the ground with a factory selected now sets its rally point, where it used to do nothing. The force-move key, which used to set it, gives the structure a move order instead. On Firestorm's mobile war factories, the only shipped structures that both take a rally point and pack up, the plain click now sets the rally point and the force-move key packs them up.

`AltToRally=yes` in `sun.ini` restores the old controls for the player who sets it.

AlexB is credited for the ts-patches option this follows.
