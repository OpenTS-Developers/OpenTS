---
key: IntentionalDeacceleration
summary: The speed a levitating unit sheds each frame while it is deliberately stopping.
see_also: ["Drag", "ProximityDistance", "MaxVelocityWhenFollowing", "MaxVelocityWhenPissedOff"]
when_omitted:
  kind: value
  value: "0.15"
---

The figure replaces [`Drag`](/keys/drag/) as the per-frame loss whenever the unit decides to stop rather than merely coast. That happens when a thrust or a coast brings its target inside [`ProximityDistance`](/keys/proximitydistance/), and when a coast falls below the speed figure for the mood the unit is in. Like `Drag` it is subtracted from the speed outright, and once it is as large as the speed held the motion is zeroed on the spot.

Braking ends when the unit is under a hundredth of a lepton per frame. It then steers at whatever it was heading for, or comes to rest and reclaims its cell if it has nothing left to head for. The figure therefore sets how long the pause between two thrusts lasts. A unit slowing from five leptons per frame stops in thirty-four frames at a loss of `0.15` and in five at a loss of `1.0`.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
