---
key: HoverBrake
summary: Time in minutes a hovering unit takes to close its throttle again.
see_also: [HoverAcceleration, HoverBoost]
when_omitted:
  kind: value
  value: ".03"
---

The mirror of [`HoverAcceleration`](/keys/hoveracceleration/), and like it a length of time rather than a rate. Each frame the drive lowers the throttle by `1 ÷ (HoverBrake × 900)` of full and floors it at nothing. At 900 frames to the minute the stock `.03` therefore closes a fully open throttle over 27 frames, a little under two seconds.

```ini title="rules.ini"
[General]
HoverBrake=.01   ; 9 frames to close a fully open throttle
```

The braking runs whenever the ceiling has fallen below the throttle the unit is holding. That happens once the unit comes within a cell of its destination, where the ceiling factor is halved before [`HoverBoost`](/keys/hoverboost/) multiplies it, leaving three quarters at the stock `150%`. It also happens outright, the ceiling dropping to nothing, while the drive has no power or the unit is pointed more than 45 degrees away from its next stop. For a hover unit, having no power here means it has also settled onto the ground. A unit that has just been shoved aside keeps a ceiling whatever its power and facing.
