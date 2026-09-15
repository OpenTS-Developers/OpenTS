---
key: HunterSeekerAscentSpeed
summary: Leptons a hunter seeker climbs each frame once it is airborne.
see_also: [HunterSeekerEmergeSpeed, HunterSeekerDescentSpeed, HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: "0"
---

While an airborne [hunter seeker](/systems/superweapons/#hunter-seeker) is below its flight level, it rises each frame by the smaller of this figure and the distance still to go. The stock rules use `40`, about a sixth of a cell a frame. While a drone with a target is further from its destination than [`HunterSeekerDescendProximity`](/keys/hunterseekerdescendproximity/), it scans the ground along its heading. When the highest ground it finds stands above the ground beneath it, it sets its flight level to that peak's height plus its type's [`FlightLevel`](/keys/flightlevel/). That flight level is measured from the ground under the drone rather than from the peak. A drone approaching high ground therefore climbs well past what clearing it would need. This figure decides whether the drone tops a ridge in its path or crawls up the face of it. The lift-off from the ground climbs at [`HunterSeekerEmergeSpeed`](/keys/hunterseekeremergespeed/) instead.

At `0` the step is zero, so a drone that has left the ground stays at whatever height it has reached however high the ground ahead of it stands.
