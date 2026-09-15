---
key: SpotlightAcceleration
summary: Radians per frame by which a sweeping spotlight's turn rate rises and falls.
see_also: [SpotlightSpeed, SpotlightAngle, HasSpotlight]
when_omitted:
  kind: value
  value: ".005"
---

Inside its arc a sweeping beam gains this much turn rate every frame for as long as the rate is below [`SpotlightSpeed`](/keys/spotlightspeed/). Once the beam has crossed half of [`SpotlightAngle`](/keys/spotlightangle/), which is the edge of its arc, the same figure comes off the rate each frame instead. The beam reverses on the frame the rate reaches zero. It therefore sets two things: how briskly a sweep gets going, and how far past the edge of its arc the beam coasts before turning back. A smaller figure gives a gentler start and a longer overshoot.

A beam [set to circle its structure or follow a target](/mapping/actions/taction-change-spotlight-behavior/) does not read it. Only a structure whose type sets [`HasSpotlight=yes`](/keys/hasspotlight/) casts a beam.

```ini title="rules.ini"
[General]
SpotlightAcceleration=.001  ; a gentler ramp than the .0025 the shipped rules set
```
