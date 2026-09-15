---
key: NavComQueueLineColor
summary: The color of the queued-destination lines, as red, green and blue from 0 to 255.
see_also: ["system:action-lines", ShowNavComQueueLines, NavComQueueLineDropShadowColor, MovementLineColor]
when_omitted:
  kind: value
  value: "74,77,255"
---

The three numbers are separated by commas; a value that is not three numbers keeps the default, a light blue. The squares on the lines' ends take the same color. The lines run through a selected object's queued destinations beyond its movement line; [Action lines](/systems/action-lines/) covers when they are drawn.

```ini title="UI.INI"
[Ingame]
NavComQueueLineColor=0,200,255
```
