---
title: Bound the weapon a Do Explosion At action names
category: fix
release: 0.2.0
targets:
- type: action
  id: TACTION_DO_EXPLOSION
  effect: changed
credit: [ZivDero]
---

Do Explosion At now detonates nothing when the weapon it names is not in the weapon list. The index went unchecked before, so the action read past the list.
