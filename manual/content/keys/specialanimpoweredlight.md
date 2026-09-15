---
key: SpecialAnimPoweredLight
summary: Whether the first special slot's animation is destroyed and recreated with its house's power.
see_also: ["SpecialAnim", "SpecialAnimPowered", "system:power"]
when_omitted:
  kind: value
  value: "no"
---

The flag swaps the freeze for outright destruction and recreation, and is read only with [`SpecialAnimPowered=no`](/keys/specialanimpowered/) beside it. [A powered light on any structure](/keys/specialanim/#a-powered-light-on-any-structure) covers the one route by which that pair fills a special slot on a structure that has no use for one. [`SpecialAnim`](/systems/building-animations/#where-each-setting-is-read-from) covers why it is not written in the same section as the slot's animation names.
