---
key: Acceleration
scope: global-rules
label: Jumpjet acceleration
see_also: [Speed, TurnRate, Climb]
when_omitted:
  kind: value
  value: ".25"
---

The jumpjet locomotor keeps a speed counter and moves the unit that many leptons along its current facing every game frame. This figure is how much the counter gains each frame while it is below the speed the current flight state asks for. The counter is never allowed past the ceiling in [`Speed`](/keys/speed/#scope-global-rules). There are 256 leptons to a cell and 15 game frames to the second.

Shedding speed is faster than gaining it. When the flight state asks for less than the counter holds, the counter drops by one and a half times this figure per frame instead, down to a floor of zero. A unit therefore slows into its landing more sharply than it worked up to cruising speed.

```ini title="rules.ini"
[JumpjetControls]
Acceleration=2
Speed=14
```

At these figures a jumpjet reaches full speed seven frames after it starts building any, and comes back down to a standstill in five.

:::caution[Zero leaves a jumpjet grounded in the air]
The counter is the only thing that moves the unit, and nothing else raises it. At `Acceleration=0` the counter never leaves zero, so a jumpjet climbs to its flight level, turns toward its destination and hovers there without ever setting off.
:::
