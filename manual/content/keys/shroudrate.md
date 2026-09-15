---
key: ShroudRate
summary: Game minutes between shroud regrowth passes.
see_also: ["system:map-visibility", ShroudGrow]
when_omitted:
  kind: value
  value: "4"
---

The value is converted to frames at 900 frames to the game minute, and the timer is reloaded with that figure after each pass. `ShroudRate=0` stops the regrowth entirely, exactly as [`ShroudGrow=no`](/keys/shroudgrow/) does.

```ini title="rules.ini"
[AudioVisual]
ShroudRate=2 ; the shroud creeps back one cell every two game minutes
```

The timer starts a scenario at zero, so the first eligible frame runs a pass immediately; the interval spaces only the passes after that one. A pass re-shrouds only cells that nothing watches, and lets everything the player controls look around again before it finishes, so a cell a unit still watches comes back in the same pass. On a map revealed only by the starting units, the immediate first pass therefore changes nothing visibly. Each pass costs the player one unwatched cell around the edge of the revealed area, so the value sets how fast the shroud closes in rather than how much it takes.
