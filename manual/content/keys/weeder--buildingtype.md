---
key: Weeder
scope: buildingtype
label: Weed refinery
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

The building accepts a docking request from a `Weeder=yes` UnitType while it holds nothing, and unloads it the way a refinery unloads a Tiberium harvester. The vehicle must belong to the same house as the building, or each house must declare the other an ally. A computer house admits such a type to [the base plan it generates](/systems/ai-base-building/#building-the-plan) only while the map carries a veinhole monster, so a map with no veins plans no weed refinery.
