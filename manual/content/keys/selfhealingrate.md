---
key: SelfHealingRate
summary: The interval between this type's self-healing ticks.
see_also: ["SelfHealRate", "SelfHealingStep", "SelfHealingCap", "SelfHealing", "system:repair"]
when_omitted:
  kind: value
  value: "-1"
  note: Any value below zero takes the interval from `SelfHealRate`.
---

```ini title="rules.ini"
[4TNK] ; a UnitType registered in [UnitTypes]
SelfHealingRate=.004
```

The type's own answer to [`SelfHealRate`](/keys/selfhealrate/), a fraction of a minute multiplied by 900 frames and truncated, used in place of the game-wide interval while the object is [mending itself](/systems/repair/#self-healing).

The tick still lands on the global frame counter rather than on a count of its own, so two types sharing an interval heal on the same frames. An interval that truncates below one frame is raised to one frame.
