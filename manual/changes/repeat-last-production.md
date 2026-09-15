---
title: Repeat the last completed structure, infantry, vehicle or aircraft from the keyboard
category: feature
release: 0.2.0
targets:
- type: command
  id: RepeatLastBuilding
  effect: added
- type: command
  id: RepeatLastInfantry
  effect: added
- type: command
  id: RepeatLastUnit
  effect: added
- type: command
  id: RepeatLastAircraft
  effect: added
credit: [ZivDero, CCHyper, dkeeton]
---

Four commands queue another of the last item of their kind that finished for the player: a structure, an infantry unit, a vehicle or an aircraft. The item must still be offered on the sidebar. A structure is refused while one is under construction or waits to be placed, and the other kinds queue behind whatever the factory is doing, as a click on the cameo would. All four arrive unbound and take whatever key the keyboard options assign.

CCHyper is credited for the Vinifera commands this follows, and dkeeton for the ts-patches building hotkey.
