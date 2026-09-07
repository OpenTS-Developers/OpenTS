---
key: ProduceCashStartup
summary: The credits handed to whoever captures the structure off a house that takes no part in the contest.
see_also: [ProduceCashStartupOneTime, ProduceCashAmount, Capturable, MultiplayPassive, "system:capture", "system:produce-cash"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="rules.ini"
[CAOILD]
Capturable=yes
ProduceCashStartup=1000 ; credits, paid once as the structure changes hands
```

Capturing the structure adds this many credits to the capturing house, but only when the house it came from has [`MultiplayPassive=yes`](/keys/multiplaypassive/) on its country. Taking a cash structure off a rival player transfers whatever it produces and pays no bonus.

A capturing house that is itself `MultiplayPassive=yes` is paid nothing, and so is zero or a negative figure.

The bonus is independent of the recurring payment. A type may set it alone — the tech structure worth a lump sum on capture and nothing afterwards — and it is outside [`ProduceCashBudget`](/keys/producecashbudget/), so it neither spends the allowance nor is limited by it. Without [`ProduceCashStartupOneTime=yes`](/keys/producecashstartuponetime/) it is paid afresh on every recapture off a neutral house.
