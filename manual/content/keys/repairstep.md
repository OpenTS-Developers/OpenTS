---
key: RepairStep
summary: The strength one repair step restores to a structure, vehicle or aircraft.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "5"
---

Every path that repairs something other than an infantry adds this many strength points per step: the wrench on a structure, and a service depot working on a vehicle or an aircraft. Infantry use [`IRepairStep`](/keys/irepairstep/) instead. It is also what a vehicle regains from each [Tiberium healing](/systems/repair/#self-healing) tick.

The same value sits inside the credit price of a step, as the number of steps a full repair is divided into. Raising it therefore makes each step larger, rarer in count and dearer, leaving the credits spent on a full repair roughly where they were. [The cost of one step](/systems/repair/#the-cost-of-one-step) works the arithmetic through.

:::danger[A step larger than the type's `Strength` crashes the game]
The cost formula divides the maximum strength of the object being repaired by this value in integers before it divides anything else by that result. A value of `0` divides by zero outright; one larger than that object's strength makes the term zero, and the next division ends the game. The strength that matters is the one being repaired at that moment, so the weakest structure, vehicle or aircraft in the rules sets the ceiling for this setting.
:::
