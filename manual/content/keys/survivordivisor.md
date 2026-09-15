---
key: SurvivorDivisor
summary: The divisor applied to a structure's cost when its number of survivors is worked out.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "100"
---

The count is the cost the structure's owner pays for it, multiplied by [`SurvivorRate`](/keys/survivorrate/) and divided by this figure, then clamped to between 1 and 5. The divisor is a price per survivor. At `100`, a structure yields one survivor for every hundred credits of the part of its cost that `SurvivorRate` keeps. The clamp then bites at both ends: a kept part of five times this figure or more yields five, and one below this figure yields one.

```ini title="rules.ini"
[General]
SurvivorRate=.4
SurvivorDivisor=200  ; a 1,000-credit structure yields two survivors
```

A structure that has [changed hands](/systems/capture/#what-changes-hands) doubles the divisor, halving its count for the rest of the match. It doubles once however often the structure changed hands. A value of `0` is checked before the division and produces no survivors rather than a division by zero.
