---
key: NavComQueueLineDropShadowColor
summary: The color of the shadow below the queued-destination lines, as red, green and blue from 0 to 255.
see_also: ["system:action-lines", NavComQueueLineDropShadow, NavComQueueLineColor]
when_omitted:
  kind: value
  value: "0,0,0"
---

The three numbers are separated by commas; a value that is not three numbers keeps the default, black. A number outside 0 to 255 wraps around, so 300,0,0 is read as 44,0,0. The color is used only with [`NavComQueueLineDropShadow=yes`](/keys/navcomqueuelinedropshadow/).
