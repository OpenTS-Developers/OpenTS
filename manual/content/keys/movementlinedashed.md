---
key: MovementLineDashed
summary: Draws the movement line as dashes rather than a solid line.
see_also: ["system:action-lines", MovementLineColor, MovementLineThick]
when_omitted:
  kind: value
  value: "no"
---

The movement line runs from a selected object to the end of its route. Its dashes are four pixels on and four off. They move along the line by the clock, one pixel every 128 milliseconds, so they run at the same pace whatever the game speed. [Action lines](/systems/action-lines/) covers when the line is drawn.
