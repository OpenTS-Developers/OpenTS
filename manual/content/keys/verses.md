---
key: Verses
summary: The percentage of a weapon's damage the warhead delivers against each armor class, in armor order.
see_also: ["system:ai-base-building", "system:target-selection"]
when_omitted:
  kind: value
  value: "100%,100%,100%,100%,100%"
  note: The default is applied whenever the warhead's section is present without the key, so a later file that carries the section and omits the key restores full damage against every armor class.
---

Entries are read in the [armor class](/reference/enums/armor/) order `none`, `wood`, `light`, `heavy`, `concrete`, and a plain fraction is accepted in place of a percentage. The `heavy` percentage is also what [a base defense's anti-air and anti-armor values](/systems/ai-base-building/#base-defenses) are computed from, and the `none` percentage its anti-infantry value, so lowering a defense weapon's effect against one armor class lowers the rating the computer weighs it by.

:::danger[A list of fewer than five entries stops the game as the rules are read]
The five entries are taken in turn whatever the line holds, and the reading does not stop when the line runs out: a list of one to four percentages is read past its end and the game halts there, before a match begins. Leaving the key out altogether is safe, because the whole default list is used in place of the line. It is a short line, not a missing one, that cannot be survived.
:::
