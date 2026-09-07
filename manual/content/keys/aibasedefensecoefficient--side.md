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

Scales how many base defenses a computer house playing for this side plans against its accumulated build cost while [the base plan is assembled](/systems/ai-base-building/#building-the-plan). Raising it multiplies the placeholders the plan carries; at `0` the plan reaches its extra defenses without a single interleaved one.
