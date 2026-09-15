---
key: AccelerationDuration
summary: How many frames a levitating unit's thrust keeps pushing.
see_also: ["Acceleration", "InitialBoost", "Drag", "AccelerationProbability"]
when_omitted:
  kind: value
  value: "20"
---

A thrust sets a counter to this figure and spends one of it per frame, adding [`Acceleration`](/keys/acceleration/) to the unit's velocity each time. When the counter reaches zero the unit stops pushing and starts to coast. Frames run fifteen to the second, so the stock twenty sustain a thrust for a little over a second.

A thrust in progress is cut short only by the target or destination it is aimed at coming within [`ProximityDistance`](/keys/proximitydistance/); reaching a mood's speed figure does not end it. A figure of `0` ends the thrust on the frame it began, leaving only the [`InitialBoost`](/keys/initialboost/) behind.

:::caution[A negative figure leaves a wandering unit adrift]
The counter is spent only while it stands above zero, and the coast begins only when it is exactly zero, so a negative figure leaves the unit in a thrust it never finishes. The thrust adds no speed, and [`Drag`](/keys/drag/) coasts the unit to a standstill. With nothing to steer at, nothing can come within proximity to end the state; only a blocked move returns the unit to the recentering path. Until then it never comes to rest or reclaims its cell.
:::

[`Drag`](/keys/drag/) covers which objects read this section and the `[General]` section a file must contain for any of it to be read at all.
