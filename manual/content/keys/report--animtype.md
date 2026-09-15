---
key: Report
scope: animtype
label: Animation sound
see_also: ["StartSound", "ExpireSound", "BounceSound"]
when_omitted:
  kind: value
  value: none
---

The sound plays once at the animation's position when the animation starts. That is not always the moment it is created: an animation asked for with a delay stays silent until the delay runs out and it begins. An animation that chains into another through [`Next`](/keys/next/) starts again as the new type and plays that type's sound too.

This is the only sound an animation type plays when it starts, whatever [`StartSound`](/keys/startsound/#scope-animtype) says.

An animation placed by [Play Anim At](/mapping/actions/taction-play-anim/), and the flare that [Drop Zone Flare](/mapping/actions/taction-dz/) plants, are marked inert only after they are created. Both still play this sound on the frame they appear. The mark takes hold from then on, silencing the type an animation chains into through `Next`.

A value naming no registered sound leaves whatever was set before in place.
