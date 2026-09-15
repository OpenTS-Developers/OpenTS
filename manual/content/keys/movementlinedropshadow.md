---
key: MovementLineDropShadow
summary: Draws a shadow below the movement line.
see_also: ["system:action-lines", MovementLineDropShadowColor, MovementLineThick]
when_omitted:
  kind: value
  value: "no"
---

The shadow is drawn first, in [`MovementLineDropShadowColor`](/keys/movementlinedropshadowcolor/), one row below the line, or two rows below a [thick](/keys/movementlinethick/) one. The squares on the line's ends gain a border of the same color. The movement line runs from a selected object to the end of its route; [Action lines](/systems/action-lines/) covers when it is drawn.
