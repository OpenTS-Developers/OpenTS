---
key: AITriggerTrackRecordCoefficient
summary: The multiplier applied to an AI trigger's history term when one of its teams fails.
see_also: ["system:ai-team-production", AITriggerFailureWeightDelta, AITriggerSuccessWeightDelta]
when_omitted:
  kind: value
  value: "1"
---

The history term is the trigger's successes so far minus half its runs so far. This value scales that term on the [failure path only](/systems/ai-team-production/#the-track-record); the success path uses the same term unscaled. The scaled term is then clamped to at most zero, so raising the value deepens the penalty for a trigger with a poor record, and does nothing once its record reaches one success in two. At `0` a failure moves the weight by [`AITriggerFailureWeightDelta`](/keys/aitriggerfailureweightdelta/) alone, regardless of history.
