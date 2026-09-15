---
key: DumbTargetStrengthCoefficient
summary: The house-wide stand-in for TargetStrengthCoefficient on a house that owns no threat rating node.
see_also: ["system:target-selection"]
no_effect: true
when_omitted:
  kind: value
  value: "0"
---

Threat scoring reads five coefficients from the type of the object doing the choosing. This house-wide set was meant to replace all five for a house that owns no threat rating structure. The two sets are all-or-nothing: no coefficient falls back on its own. Every house has the threat rating flag set as it is created, and nothing ever clears it, so the substitution never happens and [`TargetStrengthCoefficient`](/keys/targetstrengthcoefficient/) is always the value used.
