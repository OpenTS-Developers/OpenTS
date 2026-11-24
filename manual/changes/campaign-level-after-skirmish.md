---
title: Start a campaign after a skirmish or multiplayer game at level 1
category: fix
release: 0.3.0
targets:
- type: system
  id: campaign-progression
  effect: changed
credit:
- ZivDero
---

A campaign started after a LAN game began at a campaign level number taken from the LAN map's position in the map list, and one started after a skirmish kept the number an earlier campaign had raised. The number decides whether the animated introduction plays and which tech level the campaign's houses get by default. A skirmish or multiplayer game now sets the number back to 1.
