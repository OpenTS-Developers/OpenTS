---
title: Keep aircraft cargo aboard over buildings
category: fix
release: 0.1.0
targets:
- type: system
  id: aircraft-operations
  effect: changed
credit: [ZivDero, Iran]
---

A loaded aircraft now refuses self-unload while a building occupies its current cell. The check repeats when the unload mission runs. A passenger that cannot be placed is reattached at the front of the cargo hold with its transport state intact instead of being detached and lost.
