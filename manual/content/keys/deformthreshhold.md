---
key: DeformThreshhold
summary: Damage at or below this figure never craters the ground.
see_also: [Deform]
when_omitted:
  kind: value
  value: "0"
---

The comparison is strict: a blast must deal more than this figure before [`Deform`](/keys/deform/) is rolled at all, and one dealing exactly the figure is refused. The amount tested is the raw damage of the blast, not the amount any object standing in it ends up taking, so armor and distance never move a blast across the threshold.

```ini title="rules.ini"
[MyShellWH] ; example WarheadType
Deform=15%
DeformThreshhold=120 ; 121 damage and above may crater
```

The doubled `h` is part of the accepted spelling. Any other spelling is not read at all, leaving the threshold at whatever it already held: `0` unless an earlier file set it, where every blast of one point or more reaches the roll.
