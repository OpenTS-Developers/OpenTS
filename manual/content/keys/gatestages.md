---
key: GateStages
summary: The number of door frames a gate animates through.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "9"
---

The door's completion fraction is scaled by this count to pick the frame to draw. The frame is clamped to the last one in the range. The count is the number of healthy door frames in the gate's artwork: with `GateStages=9`, the travel draws frames `0` through `8`, and the damaged block starts at frame `10`.

```ini title="art.ini"
[MYGATE] ; example gate, drawn from its own Image ID
GateStages=9
```

A gate at or below [`ConditionYellow`](/keys/conditionyellow/) draws from a second block of door frames offset by this count plus one, so the art needs both blocks. With `GateStages=9`, the damaged block is frames `10` through `18`, and the door animation never draws frame `9`. The second block is automatic: a gate needs no `DamagedDoor=yes`.

For a [`Gate=yes`](/keys/gate/) type the same figure plus one is the buildup step count that [`BuildupTime`](/keys/builduptime/) is divided into, in place of the frame count the buildup art would otherwise supply.
