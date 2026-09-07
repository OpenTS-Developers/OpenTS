---
key: BuildTech
summary: The buildings a computer house resolves a generic tech prerequisite to, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
  note: The generic tech prerequisite goes unsatisfied for a computer house, so every type carrying it stays out of its base plan and its base-defense candidates; player production is unaffected.
---

The first entry the country this house acts as may own is what a [`Prerequisite=TECH`](/keys/prerequisite/) resolves to while [the base plan](/systems/ai-base-building/#building-the-plan) is assembled, and that is the whole of it. Nothing is built from the list: a tech building reaches the plan through the ordinary candidate scan, so naming one here neither adds it nor keeps it out.
