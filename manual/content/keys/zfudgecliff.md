---
key: ZFudgeCliff
summary: The depth bias applied to an object standing at the foot of a cliff.
see_also: ["ZFudgeBridge", "ZFudgeColumn", "ZFudgeTunnel"]
when_omitted:
  kind: value
  value: "10"
---

The figure is multiplied by a strength taken from the ground to the south-east of the object. The product is offered as the object's depth bias. The cell one step to the south-east gives a strength of 2 when it stands four or more height levels above the object. The cell two steps to the south-east gives 1 on the same test, and that second test overrides the first when both hold. Neither applies to an object riding a bridge deck, which is never fudged for a cliff.

Only the largest of the four fudges applies at any moment, so this one has an effect only while it beats whatever [`ZFudgeBridge`](/keys/zfudgebridge/), [`ZFudgeColumn`](/keys/zfudgecolumn/) and [`ZFudgeTunnel`](/keys/zfudgetunnel/) are contributing. The winner is added to the bias the object's locomotor asks for and to a second bias the object works out for itself.
