---
key: AIHateDelays
summary: The frames a computer house waits before picking its first enemy, one entry per difficulty.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: ""
  note: The list stays empty and the difficulty slot is used to index it anyway, reading storage that was never allocated.
---

Entries run hardest game setting first, and a computer house indexes them with its own difficulty slot, which is the inverse of the setting the player chose. The countdown is seeded once, as a skirmish or multiplayer session is set up, for every non-passive computer house; a campaign never seeds it and never runs the pick. When it expires and the house still has no enemy, the nearest non-passive, undefeated house becomes one. Until a house has an enemy, [only defensive AI triggers can pass](/systems/ai-team-production/#defensive-teams-and-the-enemy).
