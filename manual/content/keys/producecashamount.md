---
key: ProduceCashAmount
summary: The credits a structure pays its owner each interval, taken from them instead when the figure is negative.
see_also: [ProduceCashDelay, ProduceCashBudget, ProduceCashStartup, Powered, "system:produce-cash"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[CAOILD]
ProduceCashAmount=100 ; credits
ProduceCashDelay=750  ; frames between payments
```

Each time the interval [`ProduceCashDelay`](/keys/producecashdelay/) sets runs out, this many credits go to whichever house owns the structure. They go straight to that house's credits: no refinery, nothing counted as harvested, no silo cap.

A negative figure takes the money instead. Spending drains the owner's credits first, then liquidates Tiberium out of the house's own structures to cover the rest, stopping at zero rather than going into debt.

Zero produces nothing. [Buildings that produce cash](/systems/produce-cash/) owns the interval, the budget, the power test, and everything else that stops a structure paying.
