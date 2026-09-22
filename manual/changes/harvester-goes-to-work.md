---
title: Send a harvester to work when it leaves a factory
category: feature
release: 0.2.0
targets:
- type: key
  id: Harvester
  scope: unittype
  effect: changed
- type: key
  id: Weeder
  scope: unittype
  effect: changed
- type: system
  id: tiberium
  effect: changed
- type: system
  id: veins
  effect: changed
credit: [ZivDero, AlexB]
---

A harvester leaving a war factory or a repair bay now starts harvesting by itself instead of stopping on the exit cell. A factory's rally point still applies: the harvester drives there first and looks for Tiberium from there.

An armed harvester now returns to harvesting on its own too. Idle away from Tiberium, it guards its area like any armed vehicle.
