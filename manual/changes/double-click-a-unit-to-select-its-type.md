---
title: Double-click a unit to select its type
category: feature
release: 0.2.0
targets:
- type: command
  id: SelectType
  effect: changed
credit: [fjlaubscher]
---

Double-clicking one of your own units now selects every on-screen unit of its type, as a press of Select Same Type does, unless `DoubleClickSelectType=no` under `[siliconsun]` in `SUN.INI`. The first click of the double-click still selects only the unit clicked. A double-click on an enemy, on empty ground, or under fog does nothing beyond the ordinary click.
