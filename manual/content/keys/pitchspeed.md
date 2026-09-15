---
key: PitchSpeed
summary: The throttle an aircraft must be above before it pitches and banks.
see_also: ["PitchAngle", "RollAngle"]
when_omitted:
  kind: value
  value: ".25"
---

The figure is a threshold on the aircraft's throttle: a fraction of its own top speed running from 0 to 1. It is not a speed in its own right, so it does not scale with [`Speed=`](/keys/speed/#scope-aircrafttype). Above the threshold the fly locomotor drops the aircraft's nose by [`PitchAngle`](/keys/pitchangle/) and, while its facing is turning, banks it by [`RollAngle`](/keys/rollangle/). At or below it the aircraft is drawn level in both axes. A figure of 1 or more holds a type level at every speed; `PitchSpeed=0` puts it into its flying attitude the moment it starts moving.

An [`IsDropship=yes`](/keys/isdropship/) type takes neither the ordinary nose-down nor the bank. For one of those the threshold is read only while the aircraft is being rocked by a nearby jolt: above it, the type's pitch angle is added to the tilt the jolt produced. The stock dropship sets `.4`.
