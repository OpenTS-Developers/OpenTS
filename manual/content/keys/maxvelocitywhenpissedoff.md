---
key: MaxVelocityWhenPissedOff
summary: The speed at which a levitating unit closing on a target gives up coasting and brakes.
see_also: ["MaxVelocityWhenFollowing", "MaxVelocityWhenHappy", "IntentionalDeacceleration", "ProximityDistance"]
when_omitted:
  kind: value
  value: "6.5"
---

This is the mood a levitating unit is in whenever it has a target, whether or not it also has a destination; the target is tested first and settles which figure applies. Once a thrust has ended and the unit is coasting, it brakes as soon as its speed has fallen *below* this figure, or as soon as the target is inside [`ProximityDistance`](/keys/proximitydistance/). Braking runs at [`IntentionalDeacceleration`](/keys/intentionaldeacceleration/) until the unit is stopped, and it then thrusts at the target again.

As on [`MaxVelocityWhenFollowing`](/keys/maxvelocitywhenfollowing/), the figure ends a coast rather than capping a speed. At the stock pair, `6.5` here against `5` there, the shorter coast is what makes a levitating unit close on something it means to attack faster than it travels anywhere else. `MaxVelocityWhenFollowing` covers why raising either figure without limit stops paying and eventually costs ground.

:::caution[A figure of zero strands the unit]
The test is a strict comparison against the speed held, so at `0` the coast never ends on its own. Drag brings the unit to a standstill and it stays there, neither braking nor thrusting again, until its target happens to fall inside `ProximityDistance` or the target is lost.
:::

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
