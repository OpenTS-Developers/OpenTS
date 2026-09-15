---
key: MaximumAIDefensiveTeams
summary: The number of defensive teams above which a house passes over its defensive AI triggers, one entry per difficulty.
see_also: ["system:ai-team-production", MinimumAIDefensiveTeams, TotalAITeamCap, IsBaseDefense]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

Entries run hardest game setting first, and a computer house indexes them with its own difficulty slot, which is the inverse of the setting the player chose. The test is strict: the house must hold more defensive teams than the entry. It is reached only while the house is [under `TotalAITeamCap`, or holds fewer defensive teams than half its team count rounded down](/systems/ai-team-production/#the-team-budget). Exceeding the entry suppresses defensive triggers for that one pass; it never deletes a team.
