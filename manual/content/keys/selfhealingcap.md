---
key: SelfHealingCap
summary: The share of maximum strength at which this type stops healing itself.
see_also: ["SelfHealCap", "SelfHealingStep", "SelfHealingRate", "SelfHealing", "ConditionYellow", "system:repair"]
when_omitted:
  kind: value
  value: "-1"
  note: Any value below zero takes the ceiling from `SelfHealCap`.
---

```ini title="rules.ini"
[4TNK] ; a UnitType registered in [UnitTypes]
SelfHealingCap=100%
```

The type's own answer to [`SelfHealCap`](/keys/selfhealcap/), used in place of the game-wide ceiling: a [self-healing](/systems/repair/#self-healing) tick is refused once the object stands above this share of its maximum strength. A percentage and a plain fraction read the same, and `100%` mends the type completely.

[`ConditionYellow`](/keys/conditionyellow/) keeps deciding when the object's damage smoke goes out, whatever this is set to.
