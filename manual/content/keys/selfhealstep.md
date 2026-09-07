---
key: SelfHealStep
summary: The strength one self-healing tick restores.
see_also: ["SelfHealingStep", "SelfHealCap", "SelfHealRate", "SelfHealing", "RepairStep", "IRepairStep", "system:repair"]
when_omitted:
  kind: value
  value: "1"
---

```ini title="rules.ini"
[General]
SelfHealStep=1
```

Every object that [mends itself](/systems/repair/#self-healing) gains this many strength points per tick, structure, vehicle, aircraft and infantry alike. A type that states [`SelfHealingStep`](/keys/selfhealingstep/) uses its own figure instead.

[`RepairStep`](/keys/repairstep/) and [`IRepairStep`](/keys/irepairstep/) reach no part of this path; the wrench, the depot and the hospital keep those two to themselves.

A resolved step below `1` is raised to `1`, as at every other repair step in the game, so `0` and negative values still heal a point at a time; `SelfHealing=no` is what switches healing off. A step larger than the damage left is clamped to the object's maximum strength.
