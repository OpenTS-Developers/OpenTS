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

The harvester truce now discounts every `HarvesterUnit` entry when it decides that a player has
been defeated, and a vehicle thief ordered onto a shielded harvester selects it rather than
capturing it. The defeat test discounted entry 0 alone, and the thief test compared the running
vehicle against a list of types, which could never match, so a thief could take what nothing
else was allowed to touch.

AlexB is credited for the ts-patches bundle that first read this list whole.
