---
key: HunterSeekerEmergeSpeed
summary: Leptons a hunter seeker climbs each frame while it is still lifting off.
see_also: [HunterSeekerAscentSpeed, HunterSeekerDescentSpeed, HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: "0"
---

A [drone placed beside its structure](/systems/superweapons/#hunter-seeker) starts its climb the moment it is given somewhere to go. Each frame it rises by the smaller of this figure and the distance still left to its flight level. The stock rules use `6`, about a fortieth of a cell a frame. The lift-off ends once it reaches that level. Every later climb uses [`HunterSeekerAscentSpeed`](/keys/hunterseekerascentspeed/) instead.

At `0` the step is zero, so the drone gains no altitude at all. The approach that takes a drone onto its target and detonates it is entered only once the drone is off the ground and no longer lifting off.
