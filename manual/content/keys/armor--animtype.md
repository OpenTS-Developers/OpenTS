---
key: Armor
scope: animtype
label: Animation armor class
no_effect: true
see_also: [Damage, Immune]
when_omitted:
  kind: value
  value: none
---

An armor class is read when a warhead's damage is scaled before it is applied, and again when a unit weighs each of its weapons against the target it is attacking. An animation arrives at neither. [`Strength`](/keys/strength/#scope-animtype) sets out every path that would have to reach it and why none does.

An animation is on the dealing end instead. [`Damage`](/keys/damage/#scope-animtype) is the figure it applies to whatever is under it, and the class weighed against the warhead there belongs to what the damage lands on.
