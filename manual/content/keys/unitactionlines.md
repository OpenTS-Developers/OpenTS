---
key: UnitActionLines
summary: Draws lines from each selected object to its target, along its route and through its queued destinations.
see_also: ["system:action-lines", AlwaysShowActionLines, ToolTips, SidebarCameoText]
when_omitted:
  kind: value
  value: "yes"
---

The lines are drawn once per rendered frame, for every selected vehicle, infantryman or aircraft that belongs to a house under the player's own control. A selected object belonging to anyone else never draws one, whatever the setting, and a structure never does. They report the orders the object is running, its target and its movement destination together with the destinations queued behind that, and are drawn over the tactical map only, after the objects themselves. [Action lines](/systems/action-lines/) covers when they appear and how `UI.INI` styles them.

The in-game game controls dialog has the same switch and writes the choice back to `sun.ini`. Changing it there takes effect on the next frame rather than at the next scenario.
