---
key: BaseBias
summary: Parsed multiplier that the engine never uses.
no_effect: true
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "1"
---

The name suggests a multiplier applied to a candidate's threat value while that candidate stands near a friendly base. Nothing reads the stored figure, and no term in the [threat score](/systems/target-selection/#the-threat-score) measures a candidate against a base at all.
