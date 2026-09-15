---
key: Reinforce
summary: Whether a team of this type keeps recruiting after it has begun to move.
see_also: ["system:ai-team-production", "system:ai-team-execution", Max]
when_omitted:
  kind: value
  value: "no"
---

Without it a team recruits only until it starts moving, and losses are never made good. With it a team that is short of full strength keeps [looking for members](/systems/ai-team-production/#recruitment) for the rest of its life, and it keeps asking the house's production for the ones it is missing. A team of any other type counts toward that demand only until it is [forced active](/systems/ai-team-execution/#the-state-flags), reaches full strength, or is set in motion.

The setting also decides whether a team can ever be pulled back off its Script. A `Reinforce=no` team stops counting as under strength the moment it first reaches full strength or is set in motion, and stays that way however many members it afterward loses. The step that sends a team back to regroup is reached only by a team that is under strength, so such a team is never sent back. A `Reinforce=yes` team falls under strength again once it is down to a third of full strength or fewer. Where its roster asks for two members or one, it falls under strength as soon as it is short of full. Either way the team is then stopped and sent to gather. That is worth setting deliberately rather than for the recruiting alone, because [a regroup rewinds the team's Script to its first line](/systems/ai-team-execution/#every-regroup-rewinds-the-script).
