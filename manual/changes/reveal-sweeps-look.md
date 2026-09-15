---
title: Give the alliance and spied-radar sweeps an ordinary look
category: fix
release: 0.2.0
targets:
- type: key
  id: AllyReveal
  effect: changed
- type: system
  id: map-visibility
  effect: changed
credit: [ZivDero]
---

The sweep that reveals a new ally's objects and the one that reveals a spied house's objects now give each object an ordinary look instead of revealing its bare `Sight=`. The height bonus and the veteran sight bonus therefore apply, and a landed aircraft shows one cell. A structure reveals from the cell it stands on rather than from its center. An object never locked to the playable area stays dark.
