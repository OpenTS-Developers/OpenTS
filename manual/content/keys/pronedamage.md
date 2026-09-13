---
key: ProneDamage
summary: The fraction of the damage a prone infantryman takes, where one is the full amount.
see_also: [Verses, Webby]
when_omitted:
  kind: value
  value: "1"
---

The scaling is applied only to an infantryman that is lying down, only to damage above zero, and only to damage that is not forced — forced damage passes at its full figure however low the fraction. The scaled figure is truncated to a whole number and then raised to at least one, so any positive hit still costs a prone soldier a point of strength.

```ini title="rules.ini"
[MyGasWH] ; example WarheadType
ProneDamage=300% ; a gas that pools where a soldier is lying
```

A value above one raises the damage instead of lowering it. The percentage form is divided by 100 as it is read, so `300%` and `3` are the same setting.

A [`Webby=yes`](/keys/webby/) warhead zeroes the damage of any soldier its web catches after this scaling has been applied, so the setting changes nothing there. A soldier with [`IsWebImmune=yes`](/keys/iswebimmune/) is not caught, and keeps the scaled figure.
