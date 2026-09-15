---
title: Filter a mixed selection by rank or health, and drop one unit from it
category: feature
release: 0.2.0
targets:
- type: command
  id: VeterancyFilter
  effect: added
- type: command
  id: VeterancyFilterAddLower
  effect: added
- type: command
  id: HealthFilter
  effect: added
- type: command
  id: HealthFilterAddLower
  effect: added
- type: command
  id: SelectOneLess
  effect: added
credit: [ZivDero, hacklex, dkeeton]
---

The veterancy filter narrows a mixed selection to its highest rank and, pressed again, moves on to the next rank of the selection it started from. The health filter does the same with the red, yellow and green condition bands, starting from the most damaged units. The add-lower forms grow a filtered selection back by one tier instead of replacing it. Select One Less drops an unarmed object such as a harvester before any armed one. With the whole selection armed, the earliest-selected object is dropped. All five arrive unbound in `KEYBOARD.INI`, and neither filter acts while a structure is being placed.

hacklex is credited for the Vinifera filters this follows and dkeeton for the ts-patches command that removes one unit.
