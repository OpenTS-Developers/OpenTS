---
key: TargetLineDropShadow
summary: Draws a shadow below the target line.
see_also: ["system:action-lines", TargetLineDropShadowColor, TargetLineThick]
when_omitted:
  kind: value
  value: "no"
---

The shadow is drawn first, in [`TargetLineDropShadowColor`](/keys/targetlinedropshadowcolor/), one row below the line, or two rows below a [thick](/keys/targetlinethick/) one, and the squares on the line's ends gain a border of the same color. The target line runs from a selected object's firing point to what it is attacking; [Action lines](/systems/action-lines/) covers when it is drawn.
