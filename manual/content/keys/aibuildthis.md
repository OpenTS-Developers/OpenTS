---
key: AIBuildThis
summary: Allows a computer house to put the BuildingType into the base plan it generates.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

The flag is read while [the base plan is assembled](/systems/ai-base-building/#building-the-plan), beside the type's [`Owner`](/keys/owner/) entry and the scenario tech level, and nowhere else. A house whose node list came with the scenario never reads it, and `AIBuildThis=no` restricts nothing a player, a trigger or a team builds.
