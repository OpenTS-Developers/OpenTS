---
key: Cost
scope: housetype
label: Country price multiplier
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1.0"
---

Everything a house of this country builds costs [the object's own `Cost=`](/keys/cost/#scope-aircrafttype) multiplied by this value, so a figure above 1 pays more. The price of repairing an object does not use the multiplier. Selling an object back refunds that same multiplied price, cut down by [`RefundPercent=`](/keys/refundpercent/) for a house a human is playing. The product is worked out once, [when the house is given its difficulty slot](/systems/difficulty/#how-the-figures-are-combined), and not per order.

```ini title="rules.ini"
[NOD]
Cost=1.25 ; NOD pays 25% more for everything it builds
```

A campaign game drops the country's contribution, so this value shapes skirmish and multiplayer games only; the [difficulty setting's own multiplier](/keys/cost/#scope-difficulty-settings) applies in both. Prices and build times are separate axes: this value changes what everything costs without changing how long it takes.
