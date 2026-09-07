---
title: Give each country its own base unit
category: feature
release: 0.2.0
targets:
- type: key
  id: BaseUnit
  effect: changed
- type: system
  id: starting-forces
  effect: changed
credit: [ZivDero, CCHyper]
---

Naming one MCV per country in `BaseUnit=` now gives each faction its own: the key takes a list,
and a house is handed the first entry the country it acts as may own, falling back to entry 0.
Every house in a match started with the same one whatever it was playing, and mods worked around
it outside the engine. A single value written as before is a list of one and behaves exactly as
it did, and an empty list, which crashed the game before a match could start, places no base
unit at all.

CCHyper is credited for Vinifera's list under the same key, which this follows.
