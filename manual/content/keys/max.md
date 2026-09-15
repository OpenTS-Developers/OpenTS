---
key: Max
summary: The number of teams of this TeamType a house may hold at once.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: "0"
---

A negative value removes the limit entirely. Any other value is compared against a count of live teams, and the comparison is not the same on both sides of team creation. An [AI trigger is rejected](/systems/ai-team-production/#which-triggers-are-eligible) when the house already owns this many teams of the type. The creation that follows compares campaign play against the teams of this type alive anywhere in the scenario, however they were raised, and every other session type against the house's own.

:::caution[A TeamType with no `Max` can never be raised]
The stored `0` is not a missing limit. It rejects the trigger on the first pass and refuses the creation as well, so a TeamType that never sets this key is unreachable through AI triggers. The value is ignored entirely by reinforcement groups, by the team a [Change team...](/mapping/missions/tmission-teamchange/) mission creates, by the attack teams an alerted house raises, and by every team a [Create Team](/mapping/actions/taction-create-team/) action raises. A TeamType raised only that way is unaffected.
:::
