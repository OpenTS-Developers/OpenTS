---
title: Keep Wakeup Group within the list of foot units
category: fix
release: 0.2.0
targets:
- type: action
  id: TACTION_WAKEUP_GROUP
  effect: changed
credit: [ZivDero, CCHyper, tomsons26]
---

Wakeup Group walked the list of infantry, vehicles and aircraft for as many entries as the list of every object holds. On a map with any structure, that walk ran past the end of the list, and what it did there depended on the memory beyond it. It now stops at the end of the list. A map without structures behaves as before.

CCHyper and tomsons26 are credited for the Vinifera fix this follows.
