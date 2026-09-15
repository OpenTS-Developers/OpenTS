---
title: Keep a passive house dark when it allies with the player
category: fix
release: 0.2.0
targets:
- type: key
  id: MultiplayPassive
  effect: changed
- type: key
  id: AllyReveal
  effect: changed
credit: [ZivDero, Rampastring]
---

Outside a campaign, an alliance with the player no longer reveals the ground around a passive house's objects. The alliance sweep that `AllyReveal` turns on revealed by hand and skipped the rule that such objects never look.
