---
key: BuildRadar
summary: The buildings a computer house resolves a generic radar prerequisite to, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
  note: The generic radar prerequisite goes unsatisfied for a computer house, so every type carrying it stays out of its base plan and its base-defense candidates; player production is unaffected.
---

The first entry the country this house acts as may own is what a [`Prerequisite=RADAR`](/keys/prerequisite/) resolves to while [the base plan](/systems/ai-base-building/#building-the-plan) is assembled, and nothing else reads the list. It has no bearing on whether a house has a radar map, which follows from the [`Radar=yes`](/keys/radar/) buildings it owns.
