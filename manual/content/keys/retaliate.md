---
key: Retaliate
summary: Whether an object in this mission fires back at whatever damages it.
see_also: ["system:target-selection"]
when_omitted:
  kind: value
  value: "yes"
---

The setting lives in a mission's own section and is read from the mission the damaged object is currently in. At `no` the retaliation tests end there: the object keeps its orders and takes the hit without acquiring the attacker.

```ini title="rules.ini"
[Sleep]
Retaliate=no
```

A [`Veinhole=yes`](/keys/veinhole/) warhead is the one exception. It provokes a response before this test is reached, so an object in a mission marked `Retaliate=no` still turns on a veinhole monster. The exception lapses for a human-owned object that already travels under orders.
