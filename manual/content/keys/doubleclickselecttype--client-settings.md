---
key: DoubleClickSelectType
scope: client-settings
label: Double-click select type
summary: Selects every on-screen unit of one type when that unit is double-clicked.
when_omitted:
  kind: value
  value: "yes"
---

`DoubleClickSelectType=yes` selects every on-screen unit of one type when you double-click one of your own, the same selection as Select Same Type. The first click of the double-click still selects only that unit. A double-click on an enemy, on empty ground, or under fog does nothing beyond the ordinary click.

`no` leaves the double-click as an ordinary click. The Select Same Type hotkey is unaffected.

The key is read from `[siliconsun]` in `SUN.INI` at startup. No dialog offers it. Leaving the options menu writes the value currently in force.
