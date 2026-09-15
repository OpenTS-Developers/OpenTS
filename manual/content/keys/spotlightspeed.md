---
key: SpotlightSpeed
summary: Turn rate a spotlight beam builds toward, in radians per frame.
see_also: [SpotlightAcceleration, SpotlightAngle, HasSpotlight]
when_omitted:
  kind: value
  value: ".05"
---

A sweeping beam adds [`SpotlightAcceleration`](/keys/spotlightacceleration/) to its turn rate every frame for as long as the rate is still below this figure, in whichever direction it is traveling. The test is made before the step, so the rate settles at the first value that reaches or passes this figure. It can end up one acceleration step beyond it: with both keys left at their defaults, the rate settles at `.055`.

A beam [set to circle its structure](/mapping/actions/taction-change-spotlight-behavior/) reads the figure differently. It reads neither the acceleration nor [`SpotlightAngle`](/keys/spotlightangle/), and advances four times this figure in radians every frame, wrapping at a full turn. At the `.015` the shipped rules set, such a beam comes round in about a hundred and five frames. The beam itself belongs to a structure whose type sets [`HasSpotlight=yes`](/keys/hasspotlight/).

```ini title="rules.ini"
[General]
SpotlightSpeed=.03  ; twice the .015 the shipped rules set
```
