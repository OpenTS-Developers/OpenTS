---
key: RatioAITriggerTeam
summary: The percent chance that a house reads its AI triggers when its team countdown expires.
see_also: ["system:ai-team-production"]
when_omitted:
  kind: value
  value: "100"
---

A roll of `1` to `100` must come out at or below the value. Failing it ends the [team pass](/systems/ai-team-production/#when-the-pass-runs) with no suggestion and restarts the countdown; nothing else is raised in the AI triggers' place, so lowering the value only makes a house raise teams less often. The [Ratio of AI trigger teams...](/mapping/actions/taction-set-ai-trigger-team-ratio/) trigger action overwrites the value while the scenario is running, and takes the value it is given without clamping it.
