---
key: ProductionAnimDamaged
summary: The animation the production slot runs while the structure is damaged.
see_also: ["ProductionAnim", "ConditionYellow"]
when_omitted:
  kind: inherited
  note: The animation ProductionAnim names.
---

At the moment a slot is filled, only a [`ConstructionYard=yes`](/keys/constructionyard/) structure picks this name, and only when its health has fallen to [`ConditionYellow`](/keys/conditionyellow/) or below. [ProductionAnim](/keys/productionanim/) covers the three other structures that run the slot, and they always fill it in its healthy form. Any structure running the slot switches to this name when its health falls to [`ConditionYellow`](/keys/conditionyellow/) or below.

A slot that names only this one and no healthy animation therefore runs nothing on those three, and nothing on a healthy construction yard.
