---
title: Offer the vertical crossing at a crossing cell
category: fix
release: 0.3.0
targets:
- type: event
  id: TEVENT_CROSS_VERTICAL
  effect: changed
- type: system
  id: trigger-springing
  effect: changed
credit:
- JoyfulShush
---

An infantryman or vehicle that finishes a move onto a cell where a horizontal and a vertical line cross now offers both crossing events. Before, at such a crossing, the vertical crossing was offered only when the last cell of the map row also lay on a vertical line, so the move usually offered only the horizontal event. The vertical crossing is now decided by the cell it entered.
