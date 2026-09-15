---
key: PreProductionAnimDamaged
summary: The animation the pre-production slot runs while the structure is damaged.
see_also: ["PreProductionAnim", "ConditionYellow"]
when_omitted:
  kind: inherited
  note: The animation PreProductionAnim names.
---

At the moment a slot is filled, only a [`ConstructionYard=yes`](/keys/constructionyard/) structure picks this name, and only when its health has fallen to [`ConditionYellow`](/keys/conditionyellow/) or below. An unloading harvester always fills the slot on the structure it docks with in its healthy form. The docked structure switches to this name when its health falls to [`ConditionYellow`](/keys/conditionyellow/) or below while the slot is running.

A slot that names only this one and no healthy animation therefore runs nothing at a dock, and nothing on a healthy construction yard.
