---
key: TargetLaserColor
summary: The color of the sighting laser, as red, green and blue from 0 to 255.
see_also: ["system:action-lines", TargetLaser, TargetLaserDropShadowColor, TargetLineColor]
when_omitted:
  kind: value
  value: "173,0,0"
---

The three numbers are separated by commas; a value that is not three numbers keeps the default, a dark red. The squares on the line's ends take the same color. The sighting laser is the line a firing vehicle with [`TargetLaser=yes`](/keys/targetlaser/) draws to where its shot is aimed; [Action lines](/systems/action-lines/) covers when it is drawn.

```ini title="UI.INI"
[Ingame]
TargetLaserColor=255,0,0
```
