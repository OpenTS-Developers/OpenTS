---
key: IRepairRate
summary: The interval between servicing steps inside a hospital, and an armory's promotion delay.
see_also: ["system:veterancy"]
when_omitted:
  kind: value
  value: ".016"
---

A [hospital](/systems/repair/#hospitals-and-armories) or an armory runs a timer while it holds an infantry. Each time that timer reaches this value multiplied by 900, the building acts, so a larger value keeps the occupant inside longer. A hospital heals one step per elapse and releases the occupant once it is undamaged; an [`Armory=yes`](/keys/armory/) building [promotes the occupant](/systems/veterancy/#promotion-without-kills) and releases it at the first elapse.

The setting is shared by both building kinds and cannot be tuned for one without the other. Vehicle service depots run the same timer off [`URepairRate`](/keys/urepairrate/) instead.
