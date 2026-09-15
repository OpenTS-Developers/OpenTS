---
key: TargetLaserDropShadowColor
summary: The color of the shadow below the sighting laser, as red, green and blue from 0 to 255.
see_also: ["system:action-lines", TargetLaser, TargetLaserDropShadow, TargetLaserColor]
when_omitted:
  kind: value
  value: "0,0,0"
---

The three numbers are separated by commas; a value that is not three numbers keeps the default, black. The color is used only with [`TargetLaserDropShadow=yes`](/keys/targetlaserdropshadow/).
