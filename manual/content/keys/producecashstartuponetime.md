---
key: ProduceCashStartupOneTime
summary: Limits a structure's capture bonus to the first time it is paid, however often the structure changes hands afterwards.
see_also: [ProduceCashStartup, Capturable, MultiplayPassive, "system:produce-cash"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[CAOILD]
Capturable=yes
ProduceCashStartup=1000
ProduceCashStartupOneTime=yes ; the bonus is paid once, not on every recapture
```

Once [`ProduceCashStartup`](/keys/producecashstartup/) has been paid for this structure, it is never paid again: recapturing it off a neutral house transfers the structure and nothing else. Left out, every capture off a neutral house pays the bonus afresh, so a structure that returns to neutral hands and is taken again pays twice.

The record is kept per structure, not per type and not per house, and it survives a save. What counts as the same structure is not always obvious — [buildings that produce cash](/systems/produce-cash/#capture) covers the case that resets it.
