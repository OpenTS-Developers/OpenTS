---
key: RollAngle
summary: The angle in degrees an aircraft banks by while it is turning.
see_also: ["PitchAngle", "PitchSpeed"]
when_omitted:
  kind: value
  value: "30"
---

The value is written in degrees and converted to radians as it is read. The fly locomotor banks the aircraft by the angle while its facing is rotating, one way for a clockwise turn and the other for a counter-clockwise one. It banks only while the aircraft is off the ground and its throttle is above [`PitchSpeed`](/keys/pitchspeed/). It levels out again the moment the turn finishes.

An [`IsDropship=yes`](/keys/isdropship/) type never banks, and neither does an aircraft that has been rocked by a nearby jolt, which is tilted sideways by the jolt instead.

:::caution[Writing `-1` is the same as leaving the key out]
The read uses `-1` as its own marker for a missing key, so `RollAngle=-1` leaves the stored 30 degrees in place. Write `RollAngle=0` to make a type turn flat.
:::
