---
title: Refuse a scenario whose terrain data is damaged
category: fix
release: 0.2.0
targets:
- type: format
  id: scenario-terrain
  effect: changed
credit:
- gunnarbeutner
---

A scenario whose `[IsoMapPack4]` or `[IsoMapPack5]` section does not decompress, expands to a length other than its block headers claim, or ends before its terminating `CELL_NONE` now stops loading and reports damaged map data, naming the section in the debug log. Before, the cells read up to that point were kept and the game started with the rest of the map missing.
