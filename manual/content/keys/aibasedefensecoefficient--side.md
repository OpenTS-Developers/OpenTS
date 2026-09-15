---
key: AIBaseDefenseCoefficient
scope: side
label: Side base defense coefficient
see_also: [AIBaseDefensePlaceholders, "system:ai-base-building"]
when_omitted:
  kind: computed
  note: The first side takes GDIBaseDefenseCoefficient and the second NodBaseDefenseCoefficient, as each rules file sets them; any other side uses 1.
---

```ini title="rules.ini"
[GDI]
AIBaseDefenseCoefficient=1.5
```

Scales the defenses interleaved into the base plan while [it is assembled](/systems/ai-base-building/#building-the-plan). Working from the plan's fourth entry, a running build cost asks for `(cost - 2000) / 1500` defenses, truncated to a whole number. This key multiplies that count: roughly one more defense per 1500 credits of planned construction once the plan passes 2000. Each defense asked for becomes a `-1` placeholder, a reserved plan slot that the [defense planner](/systems/ai-base-building/#base-defenses) later fills with a real defense building.

The difficulty slot never enters this count; it scales only the [`AIBaseDefensePlaceholders`](/keys/aibasedefenseplaceholders/) block appended after the queue, which this key does not multiply. At `0` the plan holds just that trailing block, interleaving nothing.

```ini title="plan assembly at coefficient 1.5"
; running cost 4300 -> (4300 - 2000) / 1500 = 1, times 1.5 -> 1 defense so far
; later the running cost is 6100:
;   (6100 - 2000) / 1500 = 2, times 1.5 -> 3 wanted, 1 planned
; so two more -1 placeholders precede the entry that comes next
```
