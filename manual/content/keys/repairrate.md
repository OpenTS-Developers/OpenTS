---
key: RepairRate
summary: The interval between structure repair steps, and the interval self-healing falls back to.
see_also: ["SelfHealRate", "system:repair"]
when_omitted:
  kind: value
  value: ".016"
---

```ini title="rules.ini"
[General]
RepairRate=.1   ; one repair step every 90 frames
```

The value is a fraction of a minute: the engine multiplies it by 900 frames and truncates, giving 14 frames at the default. Both paths test the global frame counter against that interval rather than counting from the moment each object started, so every [structure repair](/systems/repair/#the-repair-tick) and every [self-healing](/systems/repair/#self-healing) object on the same interval steps on the same frames.

[`SelfHealRate`](/keys/selfhealrate/) is what times the two paths apart game-wide, and [`SelfHealingRate`](/keys/selfhealingrate/) does it for one type. Without either, a structure under the wrench and an object mending itself share this one interval. A service depot times its own work with [`URepairRate`](/keys/urepairrate/) instead, and a hospital or an armory with [`IRepairRate`](/keys/irepairrate/).

:::danger[A value below one nine-hundredth crashes the game]
The interval is truncated to a whole number of frames and then used as a divisor. Anything between zero and `1/900` truncates to zero, and the first repairing structure divides by it. Self-healing raises a truncated interval back to one frame and survives such a value.
:::
