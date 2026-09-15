---
key: TotalAITeamCap
summary: The number of live teams a house may hold before its AI triggers stop springing, one entry per difficulty.
see_also: ["system:ai-team-production", MaximumAIDefensiveTeams, MinimumAIDefensiveTeams]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

Entries run hardest game setting first: a computer house takes the first entry when the player chose the hardest setting and the last when they chose the easiest. The index is the house's own [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot). The stock `14,12,10` therefore stops a house's triggers at fourteen teams on the hardest setting and ten on the easiest. The count includes every team the house owns, including the ones the scenario placed. Give one entry per difficulty; a shorter list is indexed the same way for the slots it does not cover.

:::caution[Lowering the cap starts deleting defensive teams]
A house at or above the cap does not simply stop. If its defensive teams have also reached half its team count, rounded down, the pass [deletes the oldest of them](/systems/ai-team-production/#the-team-budget). Selection then continues only if that deletion has brought the count back under the cap.
:::
