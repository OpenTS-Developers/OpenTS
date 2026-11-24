---
title: Start each game's special options from their defaults
category: fix
release: 0.3.0
targets:
- type: key
  id: BridgeDestruction
  effect: changed
- type: key
  id: DestroyableBridges
  effect: changed
- type: key
  id: HarvesterImmune
  effect: changed
- type: key
  id: TiberiumGrows
  scope: scenarios
  effect: changed
credit:
- ZivDero
---

Some game options stayed as an earlier game had left them until the game was restarted. One skirmish with bridge destruction off kept bridges indestructible in every later skirmish, a skirmish after a network game used fast Tiberium growth and that game's harvester truce, and a campaign mission that left a `[SpecialFlags]` entry out kept the previous mission's value. Each game now starts these options from their defaults.
