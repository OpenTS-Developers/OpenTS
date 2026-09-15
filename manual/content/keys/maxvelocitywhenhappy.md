---
key: MaxVelocityWhenHappy
summary: The speed above which a wandering levitating unit stops thrusting again.
see_also: ["AccelerationProbability", "MaxVelocityWhenFollowing", "MaxVelocityWhenPissedOff", "Drag"]
when_omitted:
  kind: value
  value: "4"
---

This is the wandering mood: the unit has neither a target nor a destination and is coasting. While its speed is below this figure and no thrust is already running, it rolls [`AccelerationProbability`](/keys/accelerationprobability/) each frame for a fresh thrust in a random direction. Once its speed is at or above the figure the rolls stop and it simply coasts. The figure is therefore a soft ceiling on aimless drifting: the unit can still overshoot it, because a thrust already begun is never cut short by it. A figure of `2` ends the rolls once the unit's speed passes two leptons per frame.

A unit that has slowed below a hundredth of a lepton per frame with nothing to head for comes to rest and reclaims its cell instead, whatever this figure says.

The other two moods use their figures the other way about, as the speed at which a coast is abandoned rather than continued; [`MaxVelocityWhenFollowing`](/keys/maxvelocitywhenfollowing/) covers that.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
