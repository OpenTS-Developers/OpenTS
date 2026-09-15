---
title: Queue AI vehicle factory exit destinations
category: fix
release: 0.1.0
targets:
- type: system
  id: production
  effect: changed
credit: [ZivDero, Rampastring]
---

A vehicle produced for a computer house by a factory with `WeaponsFactory=no` now keeps the adjacent exit cell as its immediate destination. The base position the house selects for it is queued behind the exit cell. The vehicle clears the factory before continuing to the position it guards.
