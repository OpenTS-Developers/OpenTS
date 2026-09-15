---
key: SpecialAnimTwoDamaged
summary: The animation the second special slot runs while the structure is damaged.
see_also: ["SpecialAnimTwo", "SpecialAnim", "ConditionYellow"]
when_omitted:
  kind: inherited
  note: The animation SpecialAnimTwo names.
---

[The damaged form](/systems/building-animations/#the-damaged-form) covers when a slot picks this name over [`SpecialAnimTwo`](/keys/specialanimtwo/). A depot asks for the healthy name at every point it [fills this slot](/keys/specialanim/#what-starts-a-special-animation). This name is reached only when the structure crosses the damage threshold while the animation is already running, never at the moment the slot is filled. The swap keeps the frame the animation had reached rather than starting over.
