---
key: NavComQueueLineDropShadow
summary: Draws a shadow below the queued-destination lines.
see_also: ["system:action-lines", ShowNavComQueueLines, NavComQueueLineDropShadowColor, NavComQueueLineThick]
when_omitted:
  kind: value
  value: "no"
---

The shadow is drawn first, in [`NavComQueueLineDropShadowColor`](/keys/navcomqueuelinedropshadowcolor/), one row below each line, or two rows below a [thick](/keys/navcomqueuelinethick/) one, and the squares on the lines' ends gain a border of the same color. The lines run through a selected object's queued destinations beyond its movement line; [Action lines](/systems/action-lines/) covers when they are drawn.
