---
key: AttackDelay
summary: Parsed multiplier on a house's first-attack countdown, which no decision reads.
no_effect: true
see_also: ["system:ai-team-production", AttackInterval, TeamDelays]
when_omitted:
  kind: value
  value: "5"
---

Every house, computer or human, is given a countdown as it is created: a random 450 to 1800 frames multiplied by this figure and truncated to a whole number. The countdown then runs down untouched. No gameplay path reads it, so raising or lowering the multiplier changes nothing about when a computer house first attacks; that follows from [the AI trigger pass](/systems/ai-team-production/#when-the-pass-runs) and the teams it raises.
