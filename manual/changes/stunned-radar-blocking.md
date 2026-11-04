---
title: Stop a stunned radar structure from blocking a working one
category: fix
release: 0.2.0
targets:
- type: key
  id: Radar
  effect: changed
- type: system
  id: power
  effect: changed
credit:
- JoyfulShush
---

A house that relies on its structures for the radar map now looks past a stunned one, so a working `Radar=yes` structure still supplies the map. The check used to stop at the structure it found first, so a stunned radar left the map down even while a working radar stood beside it.
