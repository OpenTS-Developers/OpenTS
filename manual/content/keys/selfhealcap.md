---
key: SelfHealCap
summary: The share of maximum strength at which self-healing stops.
see_also: ["SelfHealingCap", "SelfHealStep", "SelfHealRate", "SelfHealing", "ConditionYellow", "system:repair"]
when_omitted:
  kind: value
  value: "-1"
  note: Any value below zero leaves the ceiling on `ConditionYellow`, where it was before this key existed.
---

```ini title="rules.ini"
[General]
SelfHealCap=50%
```

A tick is refused once the object stands above this share of its maximum strength, so [self-healing](/systems/repair/#self-healing) ends one step past it. A percentage and a plain fraction read the same. A type that states [`SelfHealingCap`](/keys/selfhealingcap/) uses its own figure instead.

Left alone the ceiling is [`ConditionYellow`](/keys/conditionyellow/), half strength at the engine default. `100%` mends an object completely, and nothing heals past maximum strength whatever the cap says.

`ConditionYellow` still decides when the damage smoke goes out, so a cap below it leaves an object smoking at its ceiling and one above it puts the smoke out partway up.
