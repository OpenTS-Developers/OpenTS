---
title: Require mutual alliances for refinery docking
category: fix
release: 0.1.0
targets:
- type: system
  id: tiberium
  effect: changed
- type: system
  id: veins
  effect: changed
credit: [ZivDero, dkeeton]
---

A Tiberium harvester or vein harvester can enter and dock at another house's matching refinery only when both houses declare each other allies. The permission is checked both while entering the refinery bib and when the building answers the docking request, so a unilateral alliance can no longer reserve or block another house's refinery.

Harvesters still choose their automatic return targets from the `Dock` list, among their own house's buildings only. The list does not grant compatibility with another kind of building.
