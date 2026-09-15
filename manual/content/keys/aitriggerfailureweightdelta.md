---
key: AITriggerFailureWeightDelta
summary: The amount added to an AI trigger's weight each time one of its teams fails.
see_also: ["system:ai-team-production", AITriggerSuccessWeightDelta, AITriggerTrackRecordCoefficient]
when_omitted:
  kind: value
  value: "-1"
---

The value is added, not subtracted, so a weight is only driven down by a negative one. It is added to the trigger's own [history term](/systems/ai-team-production/#the-track-record), and the total is held inside the minimum and maximum weight the trigger holds. A team records a failure whenever it dies without having reached the [Success](/mapping/missions/tmission-success/) mission in its script, whatever it achieved on the way.
