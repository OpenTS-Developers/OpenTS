---
key: OnlyTargetHouseEnemy
summary: Restricts the team's scripted target choice to the owner's declared enemy.
see_also: [Suicide, "system:base-attacked", "system:target-selection"]
when_omitted:
  kind: value
  value: "no"
---

Three script missions ask the engine to pick a target for the whole team, and each of them is handed this setting. On an [Attack](/mapping/missions/tmission-attack/) mission the team leader's scan drops any candidate whose owner is not [the house's declared enemy](/systems/base-attacked/#anger-and-the-declared-enemy), in every pass the scan makes: the flying layers, the cell rings and the full-object sweep alike. The two building-with-property missions already prefer a building of the enemy house over any other non-allied one; with this set they return nothing at all rather than settling for the other house's.

:::caution[A house with no declared enemy finds nothing]
The comparison is against a house, and a house that has never been given an enemy has none to compare with, so every candidate is refused. A campaign house [starts without one](/systems/base-attacked/#picking-a-first-enemy) and gains one only once something damages it. An `OnlyTargetHouseEnemy=yes` team sent out before that happens acquires no target and moves straight on to the next line of its script.
:::

Nothing else about the team is narrowed. Individual members still acquire their own targets by [scanning](/systems/target-selection/#when-an-object-scans) and still retaliate, both without regard to which house they are shooting at.
