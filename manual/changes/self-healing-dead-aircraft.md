---
title: Stop a self-healing aircraft from reviving in mid-air
category: fix
release: 0.2.0
targets:
- type: system
  id: repair
  effect: changed
credit: [ZivDero, JoyfulShush]
---

A self-healing aircraft shot down in flight healed itself on the way down, climbed back to its flight level, and did the same again the next time it was shot down. It now falls and dies like any other.

No shipped type reaches this; rules that give an aircraft `SelfHealing=yes` or the `SELF_HEAL` ability do.
