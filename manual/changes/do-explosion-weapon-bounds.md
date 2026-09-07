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

Do Explosion At now detonates nothing when the position it names is at or beyond the end of the weapon list. The position went unchecked before, so the action read past the list.
