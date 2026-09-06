---
title: Load structures owned by the local player in multiplayer
category: fix
release: 0.2.0
targets:
- type: format
  id: scenario-objects
  effect: changed
credit:
- ZivDero
- Iran
---

A skirmish or multiplayer scenario no longer skips a `[Structures]` row whose owner resolves to the house the local machine plays. Each machine loaded a different set of structures whenever a map owned one by a country a person was playing, and a spawn house's structures could never reach their owner.

Iran is credited for the CnCNet spawner patch that lifts the same check.
