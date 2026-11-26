---
title: A structure destroyed during a sale leaves no extra survivors
category: fix
release: 0.3.0
targets:
- type: system
  id: destruction-and-debris
  effect: changed
credit:
- JoyfulShush
---

A structure being sold could release more survivors from its destruction, on top of any survivors the sale had already released. It now releases none from the destruction, so a sale releases its survivors at most once.
