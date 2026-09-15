---
key: DumbMyEffectivenessCoefficient
summary: The house-wide stand-in for MyEffectivenessCoefficient on a house that owns no threat rating node.
see_also: ["system:target-selection"]
no_effect: true
when_omitted:
  kind: value
  value: "0"
---

Threat scoring reads five coefficients from the type of the object doing the choosing. This house-wide set was meant to replace all five for a house that owns no threat rating structure, and the two sets are all-or-nothing: there is no per-coefficient fallback between them. Every house has the threat rating flag set as it is created, and nothing ever clears it. The substitution therefore never happens, and [`MyEffectivenessCoefficient`](/keys/myeffectivenesscoefficient/) is always the value used.
