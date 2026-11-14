---
title: Stop a skirmish keeping options from an earlier game
category: fix
release: 0.2.0
targets:
- type: key
  id: BridgeDestruction
  effect: changed
- type: key
  id: HarvesterImmune
  effect: changed
- type: key
  id: TiberiumGrows
  scope: scenarios
  effect: changed
credit:
- tinix0
---

A skirmish with the bridge destruction option on now lets blasts bring bridges down. After a skirmish or network game in the same session had the option off, bridges used to stay indestructible in later skirmishes until a network game turned the option back on or the game was restarted.

A skirmish set up from the menu now always has the harvester truce off and grows Tiberium at each type's full `Growth` delay. After a network game in the same session, it used to take that game's truce and the fast Tiberium growth every network game uses.
