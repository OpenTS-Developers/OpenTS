---
title: Resolve every role list against the country a house acts as
category: fix
release: 0.2.0
targets:
- type: key
  id: ActsLike
  effect: changed
- type: key
  id: HarvesterUnit
  effect: changed
- type: key
  id: BuildRefinery
  effect: changed
- type: key
  id: BuildWeapons
  effect: changed
- type: key
  id: BuildPower
  effect: changed
- type: key
  id: BuildBarracks
  effect: changed
- type: key
  id: BuildRadar
  effect: changed
- type: key
  id: BuildTech
  effect: changed
- type: key
  id: ConcreteWalls
  effect: changed
- type: key
  id: EWGates
  effect: changed
- type: key
  id: NSGates
  effect: changed
- type: system
  id: ai-base-building
  effect: changed
credit: [ZivDero, AlexB]
---

Every list that names the types filling a role is now resolved through the country the house
acts as, the same ownership bit its construction yard produces against, so that a scenario
handing a house another country hands it that country's buildings and units as well. Ownership
had been asked two ways: a house was planned and handed what its own country may own, while
what it was then allowed to put up was tested against the country it acts as, so a campaign
house acting as another country could be planned a base out of types it could never build.

AlexB is credited for the ts-patches bundle, which resolves its picks the same way.
