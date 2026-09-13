---
key: Strength
scope: aircrafttype
label: Maximum strength
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[GAPOWR]
Strength=800
```

This scope sets the type's maximum-strength value. A damageable runtime instance created from the type starts with that value as its current strength, and damage reduces the current value.
