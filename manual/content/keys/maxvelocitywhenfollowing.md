---
key: MaxVelocityWhenFollowing
summary: The speed at which a levitating unit heading for a destination gives up coasting and brakes.
see_also: ["MaxVelocityWhenPissedOff", "MaxVelocityWhenHappy", "IntentionalDeacceleration", "ProximityDistance"]
when_omitted:
  kind: value
  value: "5"
---

This is the mood a levitating unit is in while it has a destination and no target. Once a thrust has ended and the unit is coasting, it brakes as soon as its speed has fallen *below* this figure, or as soon as the destination is inside [`ProximityDistance`](/keys/proximitydistance/). Braking runs at [`IntentionalDeacceleration`](/keys/intentionaldeacceleration/) until the unit is stopped, and it then thrusts at the destination again.

The name reads as a ceiling and the figure does not work like one. Nothing clamps a levitating unit's speed. A higher figure ends the coast sooner and brings the next thrust forward. Braking is three times steeper than the coast, so the gain runs out: past the speed a thrust reaches, the coast disappears entirely and the unit covers less ground than it would at a middling figure.

A unit that also has a target uses [`MaxVelocityWhenPissedOff`](/keys/maxvelocitywhenpissedoff/) instead; the target is tested first.

:::caution[A figure of zero strands the unit]
The test is a strict comparison against the speed held, so at `0` the coast never ends on its own. Drag brings the unit to a standstill and it stays there, neither braking nor thrusting again, until its destination happens to fall inside `ProximityDistance` or something clears the destination outright.
:::

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
