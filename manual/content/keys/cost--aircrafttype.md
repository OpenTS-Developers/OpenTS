---
key: Cost
scope: aircrafttype
label: Object price
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "0"
---

The credits charged to produce one object of the type, before the country and difficulty multipliers scale it. Three other figures are derived from it: the [base build time](/keys/buildspeed/), the [experience a kill is worth](/systems/veterancy/#earning-experience), and the [price of one repair step](/systems/repair/#the-cost-of-one-step).

Repair works from a reduced figure. A structure that hands something out has that removed first — the price of its [`FreeUnit`](/keys/freeunit/), and, on the structure the first entry of [`PadAircraft`](/keys/padaircraft/) docks at, the average price of the first two pad aircraft unless [`SeparateAircraft=yes`](/keys/separateaircraft/). Production adds both back, so the deduction shows up in repair bills alone.
