---
key: ShowNavComQueueLines
summary: Draws the destinations queued behind a selected object's movement line as further lines.
see_also: ["system:action-lines", AlwaysShowActionLines, NavComQueueLineColor]
when_omitted:
  kind: value
  value: "yes"
---

The lines continue from the end of the movement line through each destination queued with the [queue-move key](/systems/action-lines/#when-the-lines-are-shown), in the order they will be traveled. A looping queue draws a closed ring. With `no` only the movement line to the end of the current route is drawn. [Action lines](/systems/action-lines/) covers when the lines are drawn.

```ini title="UI.INI"
[Ingame]
ShowNavComQueueLines=no
```
