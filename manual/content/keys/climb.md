---
key: Climb
summary: The leptons of altitude a jumpjet unit gains or loses each game frame.
see_also: [CruiseHeight, WobbleDeviation]
when_omitted:
  kind: value
  value: "5"
---

Every frame the locomotor compares the unit's height above the ground with the height it wants. It moves the unit one step of this size in whichever direction closes the gap. The step is in leptons, 256 to a cell, and it is the same going up as coming down. At the default figure a jumpjet takes 80 frames, a little over five seconds, to climb the default [`CruiseHeight`](/keys/cruiseheight/) of `400`.

The step never overshoots downward: a descent that would put the unit below the ground stops at ground level instead. There is no such guard climbing, so the unit can rise a step past its flight level and be brought back on the following frame. That overshoot is one of the things that keeps a hovering jumpjet visibly unsettled.

```ini title="rules.ini"
[JumpjetControls]
Climb=5
```

The figure also governs how quickly a jumpjet lifts over the terrain and structures it passes across. The height it wants is raised to clear whatever is in the cell ahead. A small figure leaves a fast unit climbing after it has already reached the obstacle.
