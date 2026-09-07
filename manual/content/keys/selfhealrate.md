---
key: SelfHealRate
summary: The interval between self-healing ticks.
see_also: ["SelfHealingRate", "SelfHealStep", "SelfHealCap", "SelfHealing", "RepairRate", "system:repair"]
when_omitted:
  kind: value
  value: "-1"
  note: Any value below zero leaves self-healing on `RepairRate`, where it was before this key existed.
---

```ini title="rules.ini"
[General]
SelfHealRate=.016
```

The value is a fraction of a minute, multiplied by 900 frames and truncated, giving 14 frames at the [`RepairRate`](/keys/repairrate/) default it falls back to. Stating it takes [self-healing](/systems/repair/#self-healing) off the wrench's interval, which the two otherwise share. A type that states [`SelfHealingRate`](/keys/selfhealingrate/) uses its own figure instead.

Like the game's other repair intervals it is tested against the global frame counter rather than counted from the moment an object was hurt, so everything healing on the same interval steps on the same frames.

An interval that truncates below one frame is raised to one frame, whichever setting supplied it, so a value under `1/900` heals once per frame here instead of dividing by zero. The structure repair tick has no such floor and still [crashes on one](/keys/repairrate/).
