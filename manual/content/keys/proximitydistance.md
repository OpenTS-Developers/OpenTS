---
key: ProximityDistance
summary: How close a levitating unit must be to what it is heading for before it drifts instead of thrusting.
see_also: ["IntentionalDriftVelocity", "IntentionalDeacceleration", "MaxVelocityWhenFollowing", "MaxVelocityWhenPissedOff"]
when_omitted:
  kind: value
  value: "1.5"
---

The distance is written in cells and measured flat, between the unit's center and the point it is steering at; height is not taken into account. Inside it the unit stops using thrusts and moves at [`IntentionalDriftVelocity`](/keys/intentionaldriftvelocity/) instead. A thrust or a coast already in progress is cut short and braked at [`IntentionalDeacceleration`](/keys/intentionaldeacceleration/).

Arriving is a separate and much tighter test: the unit stops dead, with all of its motion zeroed, once it is within half a cell of the point. This figure therefore sets the width of the band in which the unit creeps up on its objective. Anything at or below half a cell removes that band entirely. A figure of `3` begins the drift while the unit is still three cells from its objective.

The same figure is used for a target and for a destination alike. For a unit steering at something, a coast ends only on this figure, on its mood's speed figure, or on a blocked move.

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
