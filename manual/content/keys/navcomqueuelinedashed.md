---
key: NavComQueueLineDashed
summary: Draws the queued-destination lines as dashes rather than solid lines.
see_also: ["system:action-lines", ShowNavComQueueLines, NavComQueueLineColor, NavComQueueLineThick]
when_omitted:
  kind: value
  value: "no"
---

The lines run through a selected object's queued destinations beyond its movement line. Their dashes are four pixels on and four off. They move along each line by the clock, one pixel every 128 milliseconds, so they run at the same pace whatever the game speed. [Action lines](/systems/action-lines/) covers when the lines are drawn.
