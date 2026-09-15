---
key: Immune
scope: animtype
label: Animation immunity
no_effect: true
see_also: [Armor, Strength]
when_omitted:
  kind: value
  value: "yes"
---

Nothing ever offers an animation to the damage step, so the flag is never tested for one. [`Strength`](/keys/strength/#scope-animtype) sets out every path that would have to reach it and why none does. Nothing removes an animation by damaging it either: one goes when its stages run out, or when whatever it was attached to goes.
