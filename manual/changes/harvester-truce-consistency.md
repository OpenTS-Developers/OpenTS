---
title: Hold the harvester truce to one list
category: fix
release: 0.2.0
targets:
- type: key
  id: HarvesterUnit
  effect: changed
- type: key
  id: HarvesterImmune
  effect: changed
credit: [ZivDero, AlexB]
---

The `HarvesterImmune` truce, a `[SpecialFlags]` entry in the map file, now discounts every `HarvesterUnit` entry. `HarvesterUnit` is the `[General]` list in `rules.ini`. The defeat test used to discount entry 0 alone. A vehicle thief ordered onto a harvester the truce protects now selects it instead of capturing it. The old thief test compared the running vehicle against a list of types, which could never match, so a thief could take what nothing else was allowed to touch.

AlexB is credited for the ts-patches bundle that first read this list whole.
