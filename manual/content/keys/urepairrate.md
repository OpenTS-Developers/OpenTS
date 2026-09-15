---
key: URepairRate
summary: The interval between the repair steps a service depot applies to the vehicle or aircraft docked with it.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: ".016"
---

The depot counts frames while it works and acts once the count reaches this value multiplied by 900, so a step falls about every 15 frames at the default. The multiplier is the game's 900 frames to the minute: the figure is a count of minutes. The count restarts after each step, and it is the only thing this value controls.

```ini title="rules.ini"
[General]
URepairRate=.033  ; about one repair step every 30 frames
```

Despite the name, it sets no step size. A depot restores [`RepairStep`](/keys/repairstep/) strength per step and charges the same credits a structure would, because vehicles and aircraft use the same figures. [One step at a time](/systems/repair/#one-step-at-a-time) traces the exchange.
