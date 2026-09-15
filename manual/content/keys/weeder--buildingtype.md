---
key: Weeder
scope: buildingtype
label: Weed refinery
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

`Weeder=yes` marks a BuildingType as a weed refinery, the structure that vein harvesters deliver to. A vein harvester is a vehicle whose UnitType sets [`Weeder=yes`](/keys/weeder/#scope-unittype). The building accepts a docking request from such a vehicle one at a time; while it is already docking or unloading a vehicle, other requests are refused. No other setting takes part in the delivery; `Weeder=yes` alone gives the building its docking behavior. The vehicle must belong to the same house as the building, or each house must declare the other an ally. The docked vehicle unloads the way a Tiberium harvester unloads at a refinery, and each load is credited to the owning house's [weed pool](/systems/veins/#the-weed-pool).

```ini title="rules.ini"
[NAWAST]        ; Tiberium Waste Facility
Weeder=yes
```

A computer house admits such a type to [the base plan it generates](/systems/ai-base-building/#building-the-plan) only while the map has a [veinhole monster](/systems/veins/#veinhole-monsters). A map with none plans no weed refinery. The gate applies only to the computer's plan; what a player builds is unaffected.
