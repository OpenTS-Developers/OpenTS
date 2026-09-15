---
key: DissolveUnfilledTeamDelay
summary: The frames an empty team outside a campaign waits before dissolving itself.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: "5000"
---

The delay is counted from the frame the team was created and applies only outside a campaign game. It is what clears away a team whose TaskForce the house never managed to fill. A team that has already reached full strength or started dissolves the moment it loses its last member, in every session type. Either dissolution is an ordinary team death, so it [records an outcome](/systems/ai-team-production/#the-track-record) against every AI trigger that names the team's type first.
