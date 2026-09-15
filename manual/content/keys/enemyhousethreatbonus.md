---
key: EnemyHouseThreatBonus
summary: Added to a candidate's threat score when the candidate belongs to the house's declared enemy.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "0"
---

The bonus is a flat addition applied once per candidate, not a multiplier, and the only term of the threat score that is read from the rules rather than from an object type. It applies when the candidate's owner is the house that the choosing object's house currently [has declared as its enemy](/systems/base-attacked/#anger-and-the-declared-enemy). A house with no declared enemy adds nothing to any candidate.

This bonus sits on the same scale as the coefficient terms, which include [`Verses`](/keys/verses/) fractions and health fractions scaled by their coefficients. The shipped value is `400`, so it competes directly with them rather than with the fixed `100000` every candidate starts from.
