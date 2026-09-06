---
title: Turn edge scrolling off with AutoScroll
category: feature
release: 0.2.0
breaking: true
migration:
- Set `AutoScroll=yes` under `[Options]` in `sun.ini` where the file already carries `AutoScroll=no`, which had no effect before and now stops edge scrolling.
targets:
- type: key
  id: AutoScroll
  effect: changed
credit:
- ZivDero
- dkeeton
---

`AutoScroll=no` now holds the tactical map still while the pointer rests against the edge of
the screen. The setting was read from `sun.ini` and written back to it, but the map scrolled
either way and nothing offered a switch.

The game controls dialog carries that switch as an Edge Scrolling check box, beside Scroll
Coasting. Keyboard scrolling, coast scrolling and the radar are unaffected.

dkeeton is credited for the ts-patches option this follows, which spells the same choice
`DisableEdgeScrolling`.
