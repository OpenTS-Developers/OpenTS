---
key: ConditionYellow
summary: The fraction of maximum strength at or below which an object counts as damaged.
see_also: ["system:power"]
when_omitted:
  kind: value
  value: ".5"
---

```ini title="rules.ini"
[AudioVisual]
ConditionYellow=.6
```

Crossing the threshold switches a structure to its damaged artwork and to the damaged form of each animation it starts. It is also the point at which a computer house's aircraft break off to look for a repair bay.

The same threshold bounds the [low-power damage tick](/systems/power/#the-structure-damage-tick), which only touches a structure standing strictly above it; a shortfall therefore grinds a base down to this fraction and stops. Raising the value shortens that decay; lowering it lets a shortfall take a base closer to destruction.
