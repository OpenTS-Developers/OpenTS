---
key: Groundspeed
scope: housetype
label: Country speed multiplier
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1.0"
---

Every driven vehicle, hovercraft, infantry unit, walker and tunneler of a house of this country has its top speed multiplied by this value, so a figure above 1 travels faster. Aircraft do not use the multiplier. The product is worked out once, [when the house is given its difficulty slot](/systems/difficulty/#how-the-figures-are-combined), and not per step.

```ini title="rules.ini"
[NOD]
Groundspeed=1.2 ; NOD's ground objects travel 20% faster
```

Outside campaign games this value, the [difficulty setting's own multiplier](/keys/groundspeed/#scope-difficulty-settings) and [`GameSpeedBias`](/keys/gamespeedbias/) are multiplied together. In a campaign game the country's value is dropped and only the difficulty setting's multiplier and `GameSpeedBias` remain.
