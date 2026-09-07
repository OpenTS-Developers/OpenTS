---
title: Name an AI trigger's side by its position
category: fix
release: 0.2.0
targets:
- type: format
  id: ai_triggers
  effect: changed
credit: [ZivDero]
---

The side field of an AI trigger record now names a position in the rules' `[Sides]` list,
counted from one, and the trigger runs for a house whose acted country belongs to that side.
`1` and `2` still mean the first and second sides, which is what the shipped AI file uses
throughout; a value naming a side the rules do not have now restricts the trigger to nothing
rather than releasing it to everyone. Data written for ts-patches or Vinifera, which compare the
field against a country's position, reads the same wherever `[Sides]` lists the sides in the
order of their countries; elsewhere rewrite each value as the position of that country's side.
