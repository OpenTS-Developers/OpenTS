---
key: ProduceCashBudget
summary: The total one structure will ever produce, counted without regard to sign, after which it stops.
see_also: [ProduceCashAmount, ProduceCashDelay, ProduceCashResetOnCapture, "system:produce-cash"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[CAOILD]
ProduceCashAmount=100
ProduceCashDelay=750
ProduceCashBudget=1000 ; ten payments, then the structure stops producing
```

Each payment [`ProduceCashAmount`](/keys/producecashamount/) makes is subtracted from this allowance, and the structure stops producing once it is gone. The amount is counted without its sign, so a drain exhausts the budget exactly as a payment does.

The last installment is paid in full rather than skipped. A budget that is not a whole multiple of the amount pays a smaller final installment, so `1000` with an amount of `300` pays 300, 300, 300 and then 100.

Zero, the default, sets no limit. A spent structure keeps standing and does nothing further unless [`ProduceCashResetOnCapture=yes`](/keys/producecashresetoncapture/) hands it a fresh allowance. The bonus [`ProduceCashStartup`](/keys/producecashstartup/) pays is outside the budget: it neither spends it nor is limited by it.
