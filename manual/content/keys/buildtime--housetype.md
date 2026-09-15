---
key: BuildTime
scope: housetype
label: Country build-time multiplier
when_omitted:
  kind: value
  value: "1.0"
---

Every object a house of this country produces has its build time multiplied by this value, so a figure above 1 builds more slowly and one below 1 faster. The product is worked out once, when the house is given its difficulty slot, and not per order.

```ini title="rules.ini"
[NOD]
BuildTime=1.2 ; NOD objects take 20% longer to build
```

Outside campaign games this value, the [difficulty setting's own multiplier](/keys/buildtime/#scope-difficulty-settings) and [`GameSpeedBias`](/keys/gamespeedbias/) are multiplied together. In a campaign game the country's value is dropped and only the difficulty setting's multiplier and `GameSpeedBias` remain.
