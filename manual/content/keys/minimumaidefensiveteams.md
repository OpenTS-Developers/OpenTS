---
key: MinimumAIDefensiveTeams
summary: The number of defensive teams a house must hold before any non-defensive AI trigger may spring, one entry per difficulty.
see_also: ["system:ai-team-production", UseMinDefenseRule, MaximumAIDefensiveTeams, IsBaseDefense]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

Entries run hardest game setting first, and a computer house indexes them with its own difficulty slot, which is the inverse of the setting the player chose. The count is the house's own running total of live defensive teams, and the comparison is skipped entirely when [`UseMinDefenseRule=no`](/keys/usemindefenserule/). Below the entry, [only a defensive trigger passes](/systems/ai-team-production/#defensive-teams-and-the-enemy). A house with no defensive trigger it can spring therefore raises nothing at all until defensive teams reach it some other way.
