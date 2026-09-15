---
title: Read EngineerDamage from its own key
category: fix
release: 0.1.0
targets:
- type: key
  id: EngineerDamage
  effect: added
credit: [ZivDero, tomsons26]
---

`[General] EngineerDamage=` is now read from its own name. The read asked the rules for
`EngineerCaptureLevel` instead, so the key was never looked for and setting the capture
level quietly filled the damage figure as well. No gameplay path reads the damage figure,
so the visible effect is that the two settings no longer contaminate one another.
