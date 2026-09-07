---
title: Name a harvester's unloading artwork per type
category: feature
release: 0.2.0
targets:
- type: key
  id: UnloadingClass
  effect: added
- type: key
  id: UnloadingHarvester
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, CCHyper]
---

A docked harvester was drawn as the one vehicle `UnloadingHarvester=` names for every harvester
in the rules, so a set of harvesters could not carry unloading artwork of their own. The new
`UnloadingClass=` names that vehicle per type and overrides the rules-wide value. A `Weeder=yes`
vein harvester, which the rules-wide value has never reached, can now be exchanged this way too.
