---
title: Ignore hostile aircraft landing reservations
category: fix
release: 0.1.0
targets:
- type: system
  id: aircraft-operations
  effect: changed
credit: [ZivDero]
---

An aircraft's landing destination now blocks another aircraft only when both belong to the same house, or their houses consider each other allied. Hostile and one-way-allied aircraft can no longer reserve a cell against one another by repeatedly assigning it as a destination. An aircraft that already occupies the cell remains a blocker.
