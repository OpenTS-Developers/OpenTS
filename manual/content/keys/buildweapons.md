---
key: BuildWeapons
summary: The war factories a computer house resolves a generic factory prerequisite to, in order of preference.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: ""
---

The first entry the country this house [acts as](/keys/actslike/) [may own](/keys/owner/) is what a [`Prerequisite=FACTORY`](/keys/prerequisite/) resolves to while [the base plan](/systems/ai-base-building/#building-the-plan) is assembled. It is also moved to second place in the candidate list. The whole list answers whether the house owns a factory at all. That answer feeds the check on whether the house can still earn, whether it keeps producing units while low on credits, and whether a house selling its base back can afford a harvester instead of a refinery. The first of those checks prices a replacement factory at the first entry the country the house acts as may own, or entry 0 when it may own none. An empty list prices it at nothing.
