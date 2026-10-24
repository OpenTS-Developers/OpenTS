---
key: EliteFlashTimer
summary: How many frames one of the local player's objects flashes after a promotion to elite.
see_also: [VeteranFlashTimer, UpgradeEliteSound, "system:veterancy"]
when_omitted:
  kind: value
  value: "0"
---

An announced promotion to elite makes the object flash for this many logic frames. The promoted object's brightness alternates while the count runs, changing every two frames. Buildings flash too.

A rise to elite from any lower rank uses this timer, including one that skips the veteran rank; [`VeteranFlashTimer`](/keys/veteranflashtimer/) is not used for it. At `0`, the promotion does not flash. The promotion's sound and EVA line still play when those are set. [Promotion feedback](/systems/veterancy/#promotion-feedback) lists the promotions that flash and the ones that stay silent.

Set the value to `0` or above. A negative value becomes a very large flash count, so the object keeps flashing after the promotion.
