---
key: BuildBarracks
summary: The barracks a computer house resolves a generic barracks prerequisite to, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
  note: The generic barracks prerequisite goes unsatisfied for a computer house, so every type with it stays out of its base plan and its base-defense candidates; player production is unaffected.
---

The first entry the country this house [acts as](/keys/actslike/) [may own](/keys/owner/) is what a [`Prerequisite=BARRACKS`](/keys/prerequisite/) resolves to while [the base plan](/systems/ai-base-building/#building-the-plan) is assembled. That entry is also moved to the head of the candidate list, so the plan queues it before the rest. Outside campaign games the whole list is read once more while the house is low on credits. A house producing units that owns none of the listed types switches back to producing structures.
