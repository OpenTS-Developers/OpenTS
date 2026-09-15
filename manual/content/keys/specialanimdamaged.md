---
key: SpecialAnimDamaged
summary: The animation the first special slot runs while the structure is damaged.
see_also: ["SpecialAnim", "ConditionYellow"]
when_omitted:
  kind: inherited
  note: The animation SpecialAnim names.
---

[The damaged form](/systems/building-animations/#the-damaged-form) covers when a slot picks this name over [`SpecialAnim`](/keys/specialanim/). A depot and a storage structure both ask for the healthy name as they [fill a special slot](/keys/specialanim/#what-starts-a-special-animation). This one is reached later, as the structure crosses the damage threshold with the animation already running, not at the moment the slot is filled.
