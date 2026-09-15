---
key: TurnRate
summary: How fast a jumpjet unit swings around to a new heading.
see_also: [Speed, Acceleration, ROT]
when_omitted:
  kind: value
  value: "3"
---

The figure is 256ths of a full rotation per game frame, the same scale an object's own [`ROT`](/keys/rot/#scope-aircrafttype) uses. A turn is timed by dividing its arc by that step, and the widest arc a single turn covers is a half circle. A full about-face therefore takes 128 divided by the figure in frames, truncated: the default `3` comes to 42 frames, a little under three seconds. It is read once, when the locomotor is created, and each jumpjet keeps the rate it was built with.

Nothing else in the flight model waits for the turn. A jumpjet begins moving toward its destination as soon as it clears a quarter of its flight level, while it is still swinging around. A low rate therefore sends it off in a wide curve rather than holding it in place.

```ini title="rules.ini"
[JumpjetControls]
TurnRate=4
```

Values are clamped at `127`, half a rotation per frame. At `0`, or at any negative figure, the facing snaps to whatever heading is asked for, with no turn at all.
