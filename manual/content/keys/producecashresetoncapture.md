---
key: ProduceCashResetOnCapture
summary: Hands each new owner of a cash-producing structure a fresh budget, reviving one that had already spent it.
see_also: [ProduceCashBudget, ProduceCashAmount, ProduceCashStartup, Capturable, "system:produce-cash"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[CAOILD]
ProduceCashAmount=100
ProduceCashDelay=750
ProduceCashBudget=1000
ProduceCashResetOnCapture=yes
```

Every capture refills the structure's [`ProduceCashBudget`](/keys/producecashbudget/) to the full figure, restarting one that had produced its allowance and stopped. Left out, the remaining allowance carries over untouched, so a spent structure is worth nothing to whoever takes it.

The reset runs on any capture, not only on one off a neutral house — unlike the bonus [`ProduceCashStartup`](/keys/producecashstartup/) pays. [Buildings that produce cash](/systems/produce-cash/#the-budget) covers what that allows.

The key does nothing without a `ProduceCashBudget` above zero, since an unlimited structure has nothing to reset.
