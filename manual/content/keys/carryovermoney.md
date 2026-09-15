---
key: CarryOverMoney
summary: The share of the previous mission's leftover money the player begins this one with.
see_also: [CarryOverCap, TimerInherit, "system:campaign-progression"]
when_omitted:
  kind: value
  value: "0"
---

```ini title="map file"
[Basic]
CarryOverMoney=0.5
CarryOverCap=-1
```

The fraction is applied to the money the player still held when the previous campaign mission was won. The product is granted as credits and folded into the house's starting credits. It is the receiving mission that decides the share, not the mission being left. A scenario reached by two different routes carries the same fraction over from either.

Anything above `1` is clamped to `1` as the value is read, so a mission cannot hand over more money than the player finished with. [`CarryOverCap`](/keys/carryovercap/) puts a ceiling on the resulting figure and, at its own default, holds it to nothing.

The carry-over is applied immediately after the next mission has loaded. A mission restarted after a loss is re-granted the same money, because the amount remembered from the previous mission is not consumed. [What survives the boundary](/systems/campaign-progression/#what-survives-the-boundary) covers what else is carried on the same terms.
