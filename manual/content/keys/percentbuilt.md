---
key: PercentBuilt
summary: Parsed base completion figure that the engine never uses.
no_effect: true
see_also: [NodeCount, "system:ai-base-building"]
when_omitted:
  kind: value
  value: "0"
---

The name promises the share of a house's prebuilt base that is already standing when the scenario opens. The engine reads the figure from the house's own section and stores it with the base's node list. A save game preserves it. How much of the plan exists at the start instead follows from the objects the scenario places; everything after that is left to the base planner. [`NodeCount`](/keys/nodecount/) covers the plan itself. On a map that sets [`UseMPAIBaseNodes=yes`](/keys/usempaibasenodes/) the figure is read from the spawn house sections as well, to the same lack of effect.
