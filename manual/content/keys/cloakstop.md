---
key: CloakStop
summary: Whether the object must come to a halt before it may start hiding.
see_also: [Cloakable, "system:cloaking"]
when_omitted:
  kind: value
  value: "no"
---

An object of this type is refused the start of a cloak while it is moving, and may start one again as soon as it stops. A cloak it already holds is never taken away by moving off, so an object that hides while stationary stays hidden for the whole journey.

Only a vehicle, infantry or aircraft reads the value. A BuildingType has no movement to test and reads it for nothing.

:::caution[A friendly field cloaks the object on the move]
The halt is tested by the object's own recloak logic alone. The cells of a [cloaking field](/systems/cloaking/#cloaking-fields) ask their occupants to hide through a route that test cannot refuse, so an object driving through its owner's field disappears without stopping. A [`Cloakable=yes`](/keys/cloakable/) object hidden that way keeps its cloak on leaving the field as well, since the only thing this key withholds is the start of a new one.
:::
