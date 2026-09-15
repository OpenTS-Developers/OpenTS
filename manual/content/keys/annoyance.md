---
key: Annoyance
summary: Makes a team that is already under way regroup when one of its members is fired on.
see_also: [Suicide, Stray, "system:base-attacked"]
when_omitted:
  kind: value
  value: "no"
---

Damage to a member is handed to its team, and what the team does with it depends on whether the team is under way. A team is under way once it has been judged fit to begin and has started working through its script. A team that is still gathering, or that has been pulled back to regroup, is not. A team that is not under way always drops its center point and goes back to regrouping and reforming, wherever the damage came from. A team that is under way normally answers by turning on the attacker instead, and this setting adds the regroup to that case: the center point is dropped and the team reforms before it presses on with its script.

Five conditions gate the moving case, and all five are already required before this setting is read. The damage must have a source that is neither allied with the team's house nor a member of the team. The member that most recently joined, not the one that was hit, must be a non-aircraft with a primary weapon. The source must not already be the team's target. If any of the five fails, a team that is under way does nothing in response: it neither regroups nor turns on the attacker.

:::caution[The regroup happens even when the target does not change]
The steps run in a fixed order, and the regroup comes first. Deciding whether to switch to the attacker starts from the team's center point, and the regroup has just emptied it, so an armed current target is always kept. A team of this type that is already shooting at something armed therefore always stops to reform and never turns on its attacker. A team with no armed current target takes the attacker as its new target, but an aircraft attacker is refused, and the old target stands.
:::

[`Suicide=yes`](/keys/suicide/) switches the whole damage response off, this setting included.
