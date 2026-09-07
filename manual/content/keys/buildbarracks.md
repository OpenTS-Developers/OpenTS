---
key: BuildBarracks
summary: The barracks a computer house resolves a generic barracks prerequisite to, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
  note: The generic barracks prerequisite goes unsatisfied for a computer house, so every type carrying it stays out of its base plan and its base-defense candidates; player production is unaffected.
---

The first entry the country this house acts as may own is what a [`Prerequisite=BARRACKS`](/keys/prerequisite/) resolves to while [the base plan](/systems/ai-base-building/#building-the-plan) is assembled, and it is also moved to the head of the candidate list so that it is planned before the rest. Outside campaign games the whole list is read once more while the house is low on credits: a house producing units that owns none of the listed types switches back to producing structures.
