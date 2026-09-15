---
key: AccelerationProbability
summary: The per-frame chance that a levitating unit with nothing to head for thrusts off in a random direction.
see_also: ["MaxVelocityWhenHappy", "AccelerationDuration", "PropulsionSoundEffect"]
when_omitted:
  kind: value
  value: "0.01"
---

The roll is made on each frame, as a fraction between zero and one, and only where the unit has neither a target nor a destination. Two places take it:

- **All of:** the unit is at rest, and its mission is neither sticky nor sleep;
- **All of:** it is coasting at a speed below [`MaxVelocityWhenHappy`](/keys/maxvelocitywhenhappy/), and no thrust is already running.

The direction is picked at random over the whole circle, so the wandering has no bias.

At the stock hundredth, an idle unit sets off roughly once every hundred frames, about seven seconds. A figure of `0` leaves a levitating unit motionless until something gives it a target or a destination. A figure of `1` puts it in a fresh thrust on every frame it is free to take one.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
