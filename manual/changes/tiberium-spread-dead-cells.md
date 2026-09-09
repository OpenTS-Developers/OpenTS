---
title: Drop a Tiberium cell that can no longer spread
category: fix
release: 0.2.0
targets:
- type: system
  id: tiberium
  effect: changed
credit:
- ZivDero
---

A cell whose Tiberium has gone since it was queued to spread is now dropped from the
spread queue. It was previously re-queued at the head of the queue whether or not it still
carried anything to seed from, so a handful of harvested cells could come back first on
every pass and leave that Tiberium type looking as though it had stopped spreading.
