---
key: BuildPDefense
summary: Parsed power-hungry defense list for a computer base that the engine never uses.
no_effect: true
see_also: ["system:ai-base-building", BuildDefense, IsBaseDefense]
when_omitted:
  kind: value
  value: ""
---

Nothing distinguishes a power-hungry defense from any other. Every candidate for a defense node is drawn the same way, from the values [the defense planner](/systems/ai-base-building/#base-defenses) reads off the types a house [may own](/keys/owner/). A planned structure's drain is weighed only when the planner decides whether to [insert a power plant ahead of it](/systems/ai-base-building/#power-and-money-interventions).
