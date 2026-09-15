---
key: NodeCount
summary: The number of base node entries read from the house's section.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "0"
---

This count sets how many node entries the engine reads. Entries are read in order from the zero-padded three-digit keys `000` upward: a `NodeCount` of `4` reads `000` through `003` and ignores `004` and everything after it. A first field matching no BuildingType ID is not reported as a problem. It produces a node of type `-1`, the base-defense placeholder the planner fills in itself. [Where the plan comes from](/systems/ai-base-building/#where-the-plan-comes-from) covers the entry format and what a supplied list suppresses. Outside a campaign the count is read from a [spawn house](/formats/scenario-objects/#spawn-houses) section, `[Spawn1]` through `[Spawn8]`, only on a map that sets [`UseMPAIBaseNodes=yes`](/keys/usempaibasenodes/).
