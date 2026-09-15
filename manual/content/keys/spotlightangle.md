---
key: SpotlightAngle
summary: Width of the arc a sweeping spotlight covers before reversing, in radians.
see_also: [SpotlightSpeed, SpotlightAcceleration, HasSpotlight]
when_omitted:
  kind: value
  value: "20"
---

A sweeping beam turns about its pivot until it has covered half this figure from the direction it started in. It then bleeds its rate off at [`SpotlightAcceleration`](/keys/spotlightacceleration/) and reverses. The other half is spent the same way on the return, so the arc is centered on the structure's facing and spans this figure end to end. A beam [set to circle its structure or follow a target](/mapping/actions/taction-change-spotlight-behavior/) does not read it. A structure has a beam to sweep only where its type sets [`HasSpotlight=yes`](/keys/hasspotlight/).

```ini title="rules.ini"
[General]
SpotlightAngle=1.05  ; a sweep about sixty degrees wide
```

:::caution[The arc is measured in radians]
The turned angle is compared against half this figure as a raw radian count, with no conversion from degrees. The shipped rules write `SpotlightAngle=.5`, an arc a little under thirty degrees wide; anything above roughly 12.6 turns the beam through more than a full circle in each direction before it reverses.
:::
