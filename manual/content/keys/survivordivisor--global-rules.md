---
key: SurvivorDivisor
scope: global-rules
label: Survivor divisor
see_also: ["system:capture", SurvivorRate, Crewed]
when_omitted:
  kind: value
  value: "100"
---

`SurvivorDivisor` works as a price per survivor. A structure yields one survivor for every `SurvivorDivisor` credits of the part of its cost that [`SurvivorRate`](/keys/survivorrate/) keeps, rounded down. That part must reach five times the divisor to yield the maximum of five survivors. A part smaller than the divisor still yields one.

```ini title="rules.ini"
[General]
SurvivorRate=.4
SurvivorDivisor=200  ; a 1,000-credit structure yields two survivors
```

A structure that has [changed hands](/systems/capture/#what-changes-hands) uses twice the divisor for the rest of the match, which roughly halves its survivors. The divisor doubles only once, however many times the structure changes hands.

A structure uses this value when the side its owner [acts as](/keys/actslike/) names no divisor, and when the owner acts as no country at all. [`SurvivorDivisor`](/keys/survivordivisor/#scope-side) covers the per-side scope.

`SurvivorDivisor=0` gives every structure that uses this value no survivors.
