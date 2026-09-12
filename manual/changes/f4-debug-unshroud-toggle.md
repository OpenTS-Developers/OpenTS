---
title: Fix the F4 playtest shroud toggle doing nothing
category: fix
release: 0.2.0
targets:
- type: command
  id: fixed:debug-unshroud
  effect: changed
credit:
- torradmin
---

Pressing F4 flagged the whole tactical view for redraw but never touched the shroud data
itself, so the map stayed exactly as shrouded as before. F4 now reveals the map when
turning the toggle on and reshrouds it when turning the toggle off, the same pair the
reveal-the-map and blackout crates use, and coordinate shroud checks now see through an
active toggle instead of only the redraw seeing it.
