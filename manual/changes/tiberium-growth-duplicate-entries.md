---
title: Queue a Tiberium cell to grow only once
category: fix
release: 0.2.0
targets:
- type: system
  id: tiberium
  effect: changed
credit:
- ZivDero
---

A cell could be queued to grow many times over, most often after a chain reaction across a
field, and every stale copy taken off the queue spent one of the pass's slots without
growing anything. Growth crawled until the pile drained. A cell now holds at most one place
in the queue.
