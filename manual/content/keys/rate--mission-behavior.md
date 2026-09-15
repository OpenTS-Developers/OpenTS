---
key: Rate
scope: mission-behavior
label: Mission servicing delay
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: ".016"
---

The value is the fraction of a minute a mission waits between one servicing pass and the next. It is read from the mission's own rules section (`[Sleep]`, `[Guard]`, `[Repair]` and the rest) and multiplied out to game frames at 900 to the minute; at the default that is 14 frames. An idle service building counts its work in these passes; [an armory's promotion clock](/systems/repair/#hospitals-and-armories) is the visible case.

```ini title="rules.ini"
[Guard]
Rate=.030  ; 27 frames between passes
```

[`AARate`](/keys/aarate/) in the same section is the counterpart used by armed buildings on guard, and it falls back to this value when it is absent or zero.
