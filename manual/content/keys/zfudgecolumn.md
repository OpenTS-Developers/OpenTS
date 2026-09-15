---
key: ZFudgeColumn
summary: The depth bias applied to an object passing a bridge support column.
see_also: ["ZFudgeBridge", "ZFudgeCliff", "ZFudgeTunnel"]
when_omitted:
  kind: value
  value: "5"
---

The figure is multiplied by a strength taken from the bridge tiles around the object. The product is offered as the object's depth bias. The strength is counted only while the object is under a bridge now, or standing where a destroyed bridge span used to pass. It counts middle span tiles, the pieces between the two ends of a span, in any state of damage. One to the south or east contributes 1, and one to the south-east adds another whether or not either of those matched. The multiplier therefore runs from 0 to 2. Only a road bridge is counted, never a railway one.

Only the largest of the four fudges applies at any moment, so this one has an effect only while it beats whatever [`ZFudgeBridge`](/keys/zfudgebridge/), [`ZFudgeCliff`](/keys/zfudgecliff/) and [`ZFudgeTunnel`](/keys/zfudgetunnel/) are contributing. The winner is added to the bias the object's locomotor asks for and to a second bias the object works out for itself. The stock rules set it on 23 vehicles, between 7 and 18.
