---
key: ConditionYellowSparkingProbability
summary: Chance each frame that a damaged object starts throwing sparks.
see_also: [ConditionRedSparkingProbability, ConditionYellow, DamageParticleSystems]
when_omitted:
  kind: value
  value: ".01"
---

The figure is a fraction from 0 to 1, drawn against once per frame, and it applies in the band between [`ConditionRed`](/keys/conditionred/) and [`ConditionYellow`](/keys/conditionyellow/). Values above `1` are accepted and start a spark on every frame the draw is made; `0` never starts one. Below `ConditionRed` the draw uses [`ConditionRedSparkingProbability`](/keys/conditionredsparkingprobability/) instead; at or above `ConditionYellow` no draw is made at all, so an undamaged object never sparks whatever this figure says.

[`ConditionRedSparkingProbability`](/keys/conditionredsparkingprobability/) covers the rest of the conditions a spark system has to meet before it starts.
