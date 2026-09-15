---
key: SuspendPriority
summary: The TeamType priority a team must reach to survive a base attack.
see_also: ["system:base-attacked", Priority, SuspendDelay]
when_omitted:
  kind: value
  value: "20"
---

When damage to one of a computer house's objects makes it call defenders back, every team that house owns is measured against this value. A team whose TeamType [`Priority`](/keys/priority/#scope-teamtype) is strictly below it has all of its members removed and is suspended, freeing them to be [called back as defenders](/systems/base-attacked/#teams-are-emptied-first). A team at or above the value keeps its members and carries on. The value is compared against the TeamType's own priority, not against any per-house or per-difficulty figure.

```ini title="rules.ini"
[General]
SuspendPriority=6 ; a team on the default Priority of 7 now survives a base attack
```

:::caution[A TeamType's default priority sits below this key's default]
A TeamType that does not state a priority is treated as `7`, which sits below this key's own default. With neither key written, every team of the attacked house is stripped whenever the house calls defenders back. Damage alone does not start that call: [six conditions refuse it](/systems/base-attacked/#when-the-call-up-is-refused) before it reaches any team. The shipped rules write `SuspendPriority=1`, so a team left on the default priority survives. Lower the threshold to the lowest priority that should survive, or give those TeamTypes a priority at or above it. Any integer is accepted, so a threshold below every priority in use suspends nothing.
:::
