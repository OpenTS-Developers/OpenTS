---
title: Require repairable structures for C4 sabotage
category: fix
release: 0.1.0
targets:
- type: system
  id: capture
  effect: changed
- type: key
  id: Repairable
  effect: changed
credit: [ZivDero, dkeeton]
---

Infantry with `C4=yes` or the C4 veteran ability now enter sabotage only for a `Repairable=yes` structure. Player clicks, attack and guard conversions, direct missions, and arrival all enforce the same requirement. A stale sabotage order against another structure is canceled without arming it.
