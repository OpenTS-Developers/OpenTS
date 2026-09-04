---
title: Keep Wakeup Group within the list of foot units
category: fix
release: 0.2.0
targets:
- type: action
  id: TACTION_WAKEUP_GROUP
  effect: changed
credit: [ZivDero]
---

Wakeup Group walked the list of infantry, vehicles and aircraft for as many entries as the
list of every object holds, so on a map with any structure it read past the end of the list
and what it did there depended on the memory beyond it. It now stops at the end of the list.
A map without structures behaves as before.
