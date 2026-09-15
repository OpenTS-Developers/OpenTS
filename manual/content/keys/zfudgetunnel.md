---
key: ZFudgeTunnel
summary: The depth bias applied to an object standing at a tunnel mouth.
see_also: ["ZFudgeBridge", "ZFudgeCliff", "ZFudgeColumn"]
when_omitted:
  kind: value
  value: "10"
---

The figure is offered as the object's depth bias whenever a tunnel mouth is in the way, and dropped otherwise. The multiplier is only ever 0 or 1, so the figure is used as written when it applies. The test first picks a tunnel cell. It takes the object's own cell if that holds a tunnel, otherwise the neighbor to the north, otherwise the one to the west. It then asks whether the cell two steps north or two steps west of the cell it picked also holds a tunnel. An object riding a bridge deck is never fudged for a tunnel.

Only the largest of the four fudges applies at any moment, so this one has an effect only while it beats whatever [`ZFudgeBridge`](/keys/zfudgebridge/), [`ZFudgeCliff`](/keys/zfudgecliff/) and [`ZFudgeColumn`](/keys/zfudgecolumn/) are contributing. The winner is added to the bias the object's locomotor asks for and to a second bias the object works out for itself. The stock rules set it on the same 23 vehicles that set a column figure, between 12 and 18.
