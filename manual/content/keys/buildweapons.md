---
key: BuildWeapons
summary: The war factories a computer house resolves a generic factory prerequisite to, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The first entry the country this house acts as may own is what a [`Prerequisite=FACTORY`](/keys/prerequisite/) resolves to while [the base plan](/systems/ai-base-building/#building-the-plan) is assembled, and it is also moved to second place in the candidate list. The whole list answers whether the house owns a factory at all, which feeds the check on whether it can still earn, whether it keeps producing units while low on credits, and whether a house selling its base back can afford a harvester instead of a refinery; the first entry the country the house acts as may own, or entry 0 when it may own none, supplies the factory price the first of those prices a replacement at, and an empty list prices it at nothing.
