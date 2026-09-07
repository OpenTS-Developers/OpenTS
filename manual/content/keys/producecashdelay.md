---
key: ProduceCashDelay
summary: The frames between one structure's cash payments, and the switch that turns them on at all.
see_also: [ProduceCashAmount, ProduceCashBudget, "system:produce-cash"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[CAOILD]
ProduceCashAmount=100
ProduceCashDelay=750  ; frames; 750 is 50 seconds at the normal game rate
```

The interval is loaded with this many frames when the structure opens for business and again after each payment, so [`ProduceCashAmount`](/keys/producecashamount/) is paid on this spacing. Each structure counts its own interval down, so two of the same type built apart pay apart.

Zero or a negative figure produces nothing at all, which makes the recurring payment opt-in: a type setting only [`ProduceCashStartup`](/keys/producecashstartup/) pays its capture bonus and nothing else, rather than paying an unset amount on every frame.

On a [`Powered=yes`](/keys/powered/) structure the count is paused rather than restarted whenever the structure cannot produce, so [restoring power](/systems/produce-cash/#power) resumes the interval where it stopped instead of paying immediately.
