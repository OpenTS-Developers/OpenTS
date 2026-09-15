---
key: ToolTips
summary: Pops the interface tooltips up while a scenario is running.
see_also: [SidebarCameoText, UnitActionLines]
when_omitted:
  kind: value
  value: "yes"
---

Tooltips are switched to this setting as the game loop is entered and switched off again as it is left. The setting therefore reaches the sidebar cameos, the tabs and the rest of the in-game interface, but never the menus or the dialogs in front of them. [`SidebarCameoText`](/keys/sidebarcameotext/) explains what a cameo's tooltip reports.

The in-game game controls dialog has the same switch and writes the choice back to `sun.ini`. Changing it there takes effect at once, but only while a scenario is running.
