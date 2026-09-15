---
key: SpecialAnimThree
summary: The animation the structure runs in its third special slot.
see_also: ["SpecialAnim", "SpecialAnimTwo", "SpecialAnimThreeDamaged", "SpecialAnimThreeX", "SpecialAnimThreeY", "SpecialAnimThreeYSort", "SpecialAnimThreeZAdjust", "SpecialAnimThreePowered", "SpecialAnimThreePoweredLight", "UnitRepair"]
when_omitted:
  kind: value
  value: ""
---

[Building animations](/systems/building-animations/) covers the slot mechanism all three share, and [what starts a special animation](/keys/specialanim/#what-starts-a-special-animation) covers which structures reach a special slot at all. The third slot belongs to a [`UnitRepair=yes`](/keys/unitrepair/) depot alone. It is created as a repair visit ends, and no part of the repair cycle stops it afterward. A later visit starts its own first-slot animation alongside it, so an animation that loops here holds the slot until the structure is sold or otherwise taken off the map. Neither the fill indicator of a [`SiloDamage=yes`](/keys/silodamage/) structure nor a [`FirestormWall=yes`](/keys/firestormwall/) section ever reaches it.
