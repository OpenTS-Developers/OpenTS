---
title: Show tooltip text as written
category: fix
release: 0.2.0
targets:
- type: system
  id: sidebar
  effect: changed
credit:
- ZivDero
---

Tooltips now show a `%` sign as written, and show long text in full. Previously a `%` in a name or `Description=` was read as a formatting code, which could print stray numbers or crash the game, and text of 256 bytes or more drew with stray characters at its end.
