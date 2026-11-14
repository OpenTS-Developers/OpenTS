---
key: ToolTips
summary: Pops the interface tooltips up while a scenario is running.
see_also: [SidebarCameoText, UnitActionLines]
when_omitted:
  kind: value
  value: "yes"
---

`ToolTips=yes` shows tooltips over the sidebar cameos, the tabs and the rest of the in-game interface while a scenario is being played. Tooltips are off in the menus, and are switched off when the in-game options open, whatever this setting says. [`SidebarCameoText`](/keys/sidebarcameotext/) describes what a cameo's tooltip shows.

A tooltip appears once the pointer has rested on a cameo, tab or other control for half a second. It goes away when the pointer leaves the control, when a mouse button is pressed, or after 10 seconds. While a tooltip is up, and for one second after one goes away, moving onto another control shows that control's tooltip at once. A tooltip that a click or the 10 seconds took down does not come back until the pointer has left its control.

Over the battlefield, a tooltip such as an enemy unit's name waits the half second every time, and goes away as soon as the pointer moves.

The game controls dialog has the same switch, and accepting the dialog saves the choice to `sun.ini`. A change made during a game takes effect at once. A change made from the main menu takes effect when the next scenario starts.
