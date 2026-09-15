---
key: TargetLineDashed
summary: Draws the target line as dashes rather than a solid line.
see_also: ["system:action-lines", TargetLineColor, TargetLineThick]
when_omitted:
  kind: value
  value: "no"
---

The target line runs from a selected object's firing point to what it is attacking. Its dashes are four pixels on and four off. They move along the line by the clock, one pixel every 64 milliseconds, so they run at the same pace whatever the game speed. [Action lines](/systems/action-lines/) covers when the line is drawn.
