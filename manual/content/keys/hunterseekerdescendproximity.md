---
key: HunterSeekerDescendProximity
summary: Horizontal distance from its destination, in leptons, at which a hunter seeker starts diving onto its target.
see_also: [HunterSeekerDetonateProximity, HunterSeekerDescentSpeed, HunterSeekerAscentSpeed, HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: "0"
---

The figure is a horizontal distance, measured flat across the map. Once a [hunter seeker](/systems/superweapons/#hunter-seeker) with a target comes nearer than this to the coordinate it is flying to, its flight level is set between the target's altitude and its type's [`FlightLevel`](/keys/flightlevel/). The stock rules use `700`, about two and three-quarter cells. The level it is told to hold moves from its cruising level towards the target's altitude in proportion to the range already flown. At the outer edge of this range the drone is still at its cruising level, and on arrival it is at the target's own altitude. The result is never allowed below 10 leptons above the ground. [`HunterSeekerDescentSpeed`](/keys/hunterseekerdescentspeed/) covers how quickly the drone actually gives up the altitude.

Further out than this, the drone instead scans ten frames of its own flight ahead along its heading. When the highest ground it finds stands above the ground beneath it, it sets its flight level to that peak's height plus its type's `FlightLevel`. Because that level is measured from the ground under the drone rather than from the peak, a drone approaching high ground climbs well past what clearing it would need. This figure is therefore the point at which terrain stops being avoided and the dive begins.

At `0` the measured distance can never fall below it, so the drone stays on the terrain-clearing path for the whole approach and never dives.
