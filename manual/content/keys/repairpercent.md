---
key: RepairPercent
summary: The share of an object's price that a full repair charges.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: ".25"
---

```ini title="rules.ini"
[General]
RepairPercent=.5   ; a full repair charges about half the object's price
```

The multiplier sits at the end of the step price, after the object's cost has been divided into steps: `(cost / (Strength / RepairStep)) * RepairPercent`, never less than one credit. It applies to both paths that charge for a repair: the wrench on a structure, and a service depot working on a vehicle or aircraft. Infantry never reach it, because a hospital is free.

The share is approximate in both directions. Both divisions and the multiplication truncate, so a structure costing 1000 with [`Strength=400`](/keys/strength/) is charged 3 credits for each of its 80 steps at the default. That is 240 credits rather than the 250 the fraction names. The one-credit floor pushes the other way on anything cheap with a large strength. A structure costing 100 with `Strength=1000` is charged the floor on all 200 of its steps, so a full repair costs twice what the structure did. [The cost of one step](/systems/repair/#the-cost-of-one-step) shows where each truncation lands.
