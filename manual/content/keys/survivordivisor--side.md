---
key: SurvivorDivisor
scope: side
label: Side survivor divisor
see_also: ["system:capture", SurvivorRate, Crewed]
when_omitted:
  kind: inherited
  note: The `[General]` `SurvivorDivisor`.
---

```ini title="rules.ini"
[Nod]
SurvivorDivisor=150
```

The divisor a structure of this side uses in place of the `[General]` [`SurvivorDivisor`](/keys/survivordivisor/#scope-global-rules) when its survivor count is worked out. [Survivors](/systems/capture/#survivors) gives the formula, the one-to-five limit and the odds for each cell.

The value applies to every structure whose owning house [acts as](/keys/actslike/) a country on this side. `0` gives those structures no survivors, and a higher value yields fewer survivors. A captured structure doubles the divisor in force, so it still counts this value doubled.

Write a number. `<none>` is not a way to clear this key: it reads as `0`, which gives those structures no survivors. A negative value leaves the `[General]` divisor in force instead.
