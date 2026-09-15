---
key: CarryOverCap
summary: The ceiling placed on the money carried into this mission from the previous one.
see_also: [CarryOverMoney]
when_omitted:
  kind: value
  value: "0"
---

```ini title="map file"
[Basic]
CarryOverMoney=0.5
CarryOverCap=5000
```

The figure is a credit total, compared against the money [`CarryOverMoney`](/keys/carryovermoney/) worked out and used in its place whenever it is smaller. A cap of exactly `-1` is the one value treated as "no ceiling" and lets the full share through. Every other negative figure is a ceiling like any other and hands the player a debt.

:::caution[The default cap cancels the carry-over]
Leaving the key out stores `0`, and a ceiling of zero holds any share to nothing. A mission that wants money carried forward has to state both keys, and a mission that wants the whole share has to write `CarryOverCap=-1`.
:::
