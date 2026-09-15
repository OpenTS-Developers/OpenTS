---
key: IsBaseDefense
scope: buildingtype
label: Base defense building
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

Only a type with this flag has its anti-air, anti-armor and anti-infantry values [computed from its primary weapon](/systems/ai-base-building/#base-defenses). A type whose three values stay at zero never enters a defense candidate list. Where the computer is not following a map plan, the node of such a building is retired to a fresh placeholder when the building is taken off the map. The planner then picks a new type and cell rather than rebuilding what was lost.
