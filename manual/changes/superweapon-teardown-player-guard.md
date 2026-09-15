---
title: Recount superweapons safely while a scenario is cleared
category: fix
release: 0.1.0
targets:
- type: system
  id: superweapons
  effect: changed
credit: [ZivDero, Rampastring]
---

Loading a scenario clears the one in play, and each house now finishes updating its superweapons after the local player has been removed. Structures are deleted after that, and their removal can suspend or restore a powered superweapon. The availability update still runs, but targeting-mode and sidebar work is skipped until there is a local player again.
