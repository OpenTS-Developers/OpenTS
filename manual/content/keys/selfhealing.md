---
key: SelfHealing
summary: Mends the object a step at a time while it is below its self-healing ceiling.
see_also: ["SelfHealStep", "SelfHealRate", "SelfHealCap", "SelfHealingStep", "SelfHealingRate", "SelfHealingCap", "system:repair"]
when_omitted:
  kind: value
  value: "no"
---

The tick restores [`SelfHealStep`](/keys/selfhealstep/) strength points on the [`SelfHealRate`](/keys/selfhealrate/) interval, and the type's own [`SelfHealingStep`](/keys/selfhealingstep/) and [`SelfHealingRate`](/keys/selfhealingrate/) come first where they are stated. State none of the four and the object heals one point every 14 frames, on [`RepairRate`](/keys/repairrate/). It costs nothing, needs no building and no order, and works the same way on structures, vehicles, aircraft and infantry.

Granting the `SELF_HEAL` ability through [`VeteranAbilities`](/keys/veteranabilities/) or [`EliteAbilities`](/keys/eliteabilities/) reaches the same path, so a promoted object heals exactly as a type with this flag does.

:::caution[Healing stops at a ceiling, not at full strength]
The tick is refused the moment the object rises above [`SelfHealCap`](/keys/selfhealcap/), or the type's [`SelfHealingCap`](/keys/selfhealingcap/), so it ends one step past that share of maximum strength. With neither stated the ceiling is [`ConditionYellow`](/keys/conditionyellow/), and the object recovers to just over half strength and stops. A ceiling of `100%` mends it completely, as [`TiberiumHeal=yes`](/keys/tiberiumheal/#scope-aircrafttype) does on its own path.
:::
