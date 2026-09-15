---
key: MovementLineColor
summary: The color of the movement line, as red, green and blue from 0 to 255.
see_also: ["system:action-lines", MovementLineDropShadowColor, TargetLineColor, NavComQueueLineColor]
when_omitted:
  kind: value
  value: "0,170,0"
---

The three numbers are separated by commas; a value that is not three numbers keeps the default, a green. The squares on the line's ends take the same color. The movement line runs from a selected object to the end of its route; [Action lines](/systems/action-lines/) covers when it is drawn.

```ini title="UI.INI"
[Ingame]
MovementLineColor=0,255,0
```
