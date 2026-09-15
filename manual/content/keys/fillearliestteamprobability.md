---
key: FillEarliestTeamProbability
summary: The percent chance that the next vehicle, infantryman or aircraft chosen goes to the oldest waiting team, one entry per difficulty.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

Entries run hardest game setting first. The shipped rules write `100,80,60`: a `100` percent chance on the hardest setting, `80` on the next and `60` on the easiest. A computer house indexes them with its own difficulty slot, the inverse of the setting the player chose. A game set to Hard gives the computer the first entry, and one set to Easy gives it the last. Vehicles, infantry and aircraft each have their own routine, and each routine makes the draw once per [production choice](/systems/ai-team-production/#production-demand). Winning it selects the type wanted by the team that has waited longest. Losing it selects uniformly from the list of candidates the same pass has gathered.
