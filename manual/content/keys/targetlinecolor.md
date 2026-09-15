---
key: TargetLineColor
summary: The color of the target line, as red, green and blue from 0 to 255.
see_also: ["system:action-lines", TargetLineDropShadowColor, MovementLineColor, TargetLaserColor]
when_omitted:
  kind: value
  value: "173,0,0"
---

The three numbers are separated by commas; a value that is not three numbers keeps the default, a dark red. The squares on the line's ends take the same color. The target line runs from a selected object's firing point to what it is attacking; [Action lines](/systems/action-lines/) covers when it is drawn.

```ini title="UI.INI"
[Ingame]
TargetLineColor=255,64,64
```
