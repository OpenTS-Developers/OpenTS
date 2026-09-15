---
title: Respect terrain immunity during vehicle pathing
category: fix
release: 0.1.0
targets:
- type: system
  id: movement-and-terrain
  effect: changed
credit: [ZivDero, Rampastring]
---

A vehicle now treats an `Immune=yes` terrain object as impassable even when its selected weapon sets `Wood=yes`. `Immune` sits on the terrain type and `Wood` on the warhead, in `rules.ini`. Non-immune terrain remains destroyable by a wood-damaging warhead, while weapons without that flag still cannot clear terrain objects.
