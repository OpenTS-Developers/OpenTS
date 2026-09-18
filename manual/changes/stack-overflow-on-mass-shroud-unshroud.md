---
title: Fix crash when large fog changes unwound recursively
category: fix
release: 0.2.0
targets:
- type: system
  id: map-visibility
  effect: changed
credit:
- torradmin
---

Regrowing fog over a cell recursed into every neighbor that also needed to regrow, so
shrouding or unshrouding a large connected area — a big map, a reveal-the-map crate, or the
[map shroud toggle](/commands/fixed-debug-unshroud/) — could recurse deeply enough to
overflow the stack and crash. Fog regrowth now walks an explicit queue instead, so the
depth of a shroud change no longer bounds how much fog can change at once.
