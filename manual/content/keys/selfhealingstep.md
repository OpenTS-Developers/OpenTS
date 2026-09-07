---
key: SelfHealingStep
summary: The strength one self-healing tick restores to this type.
see_also: ["SelfHealStep", "SelfHealingCap", "SelfHealingRate", "SelfHealing", "system:repair"]
when_omitted:
  kind: value
  value: "-1"
  note: Any value below zero takes the step from `SelfHealStep`.
---

```ini title="rules.ini"
[4TNK] ; a UnitType registered in [UnitTypes]
SelfHealingStep=15
```

The type's own answer to [`SelfHealStep`](/keys/selfhealstep/), read from any TechnoType and used in place of the game-wide step while it [mends itself](/systems/repair/#self-healing). It is the only way to single a type out: unlike [`RepairStep`](/keys/repairstep/) and [`IRepairStep`](/keys/irepairstep/), the game-wide step draws no line even between infantry and everything else.

A resolved step below `1` is raised to `1`, so `0` heals a point at a time rather than nothing, and the step is clamped to the type's [`Strength`](/keys/strength/#scope-aircrafttype). It does nothing without [`SelfHealing=yes`](/keys/selfhealing/) or the `SELF_HEAL` ability.
