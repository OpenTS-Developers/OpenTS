---
key: AITriggerSuccessWeightDelta
summary: The amount added to an AI trigger's weight each time one of its teams succeeds.
see_also: ["system:ai-team-production", AITriggerFailureWeightDelta, AITriggerTrackRecordCoefficient]
when_omitted:
  kind: value
  value: "1"
---

The delta is added alongside the trigger's own [history term](/systems/ai-team-production/#the-track-record), and the result is then held inside the minimum and maximum weight the trigger holds. Because that clamp is applied every time, a value larger than the span between a trigger's own bounds only makes it reach its maximum sooner. It cannot push the weight past that maximum.
