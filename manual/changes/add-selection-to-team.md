---
title: Add the current selection to a team
category: feature
release: 0.2.0
targets:
- type: command
  id: TeamAddTo_1
  effect: added
- type: command
  id: TeamAddTo_2
  effect: added
- type: command
  id: TeamAddTo_3
  effect: added
- type: command
  id: TeamAddTo_4
  effect: added
- type: command
  id: TeamAddTo_5
  effect: added
- type: command
  id: TeamAddTo_6
  effect: added
- type: command
  id: TeamAddTo_7
  effect: added
- type: command
  id: TeamAddTo_8
  effect: added
- type: command
  id: TeamAddTo_9
  effect: added
- type: command
  id: TeamAddTo_10
  effect: added
credit: [ZivDero, dkeeton]
---

Ten Add To Team commands select a team the way Add Select Team does, keeping whatever was
already selected, and then make that whole selection the team. Growing a control group no
longer means selecting its members again and creating it from scratch. An object that
belonged to another team leaves it, because an object holds one team at a time, and an
object in limbo, such as a passenger in a transport, is left in the team it already has.
All ten arrive unbound.

dkeeton is credited for the ts-patches command this follows.
