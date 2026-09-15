---
key: SpecialAnimTwo
summary: The animation the structure runs in its second special slot.
see_also: ["SpecialAnim", "SpecialAnimTwoDamaged", "SpecialAnimTwoX", "SpecialAnimTwoY", "SpecialAnimTwoYSort", "SpecialAnimTwoZAdjust", "SpecialAnimTwoPowered", "SpecialAnimTwoPoweredLight", "SpecialAnimThree", "UnitRepair", "FirestormWall"]
when_omitted:
  kind: value
  value: ""
---

[Building animations](/systems/building-animations/) covers the slot mechanism all three share, and [what starts a special animation](/keys/specialanim/#what-starts-a-special-animation) covers which structures reach a special slot at all. The second slot is the middle stage of a [`UnitRepair=yes`](/keys/unitrepair/) depot's sequence. It is created when the first slot's animation plays to its end, and stopped as the visit ends. A first animation that loops never reaches its end, so this one never appears.

A [`FirestormWall=yes`](/keys/firestormwall/) section takes the slot for a hard-coded animation of its own and leaves this name unused. The fill indicator of a [`SiloDamage=yes`](/keys/silodamage/) structure never reaches it.
