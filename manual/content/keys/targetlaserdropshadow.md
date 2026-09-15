---
key: TargetLaserDropShadow
summary: Draws a shadow below the sighting laser.
see_also: ["system:action-lines", TargetLaser, TargetLaserDropShadowColor, TargetLaserThick]
when_omitted:
  kind: value
  value: "no"
---

The shadow is drawn first, in [`TargetLaserDropShadowColor`](/keys/targetlaserdropshadowcolor/), one row below the line, or two rows below a [thick](/keys/targetlaserthick/) one. The squares on the line's ends gain a border of the same color. The sighting laser is the line a firing vehicle with [`TargetLaser=yes`](/keys/targetlaser/) draws to where its shot is aimed; [Action lines](/systems/action-lines/) covers when it is drawn.
