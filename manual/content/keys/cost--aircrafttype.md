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

Repair of a structure that comes with a [`FreeUnit`](/keys/freeunit/) or pad aircraft is priced from a reduced figure, which [What a structure gives away](/keys/cost/#what-a-structure-gives-away) explains; its purchase price is unchanged. An aircraft's own repair uses its full written price.
