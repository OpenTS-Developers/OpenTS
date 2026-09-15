---
key: HunterSeekerDetonateProximity
summary: Horizontal distance from its destination, in leptons, at which a hunter seeker detonates.
see_also: [HunterSeekerDescendProximity, C4Warhead, HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: "0"
---

A [hunter seeker](/systems/superweapons/#hunter-seeker) closing on a target detonates the frame it comes nearer than this to the coordinate it is flying to, measured flat across the map. The stock rules use `150`, about three-fifths of a cell. The test is made before the dive test, so a figure at or above [`HunterSeekerDescendProximity`](/keys/hunterseekerdescendproximity/) leaves no range in which the drone descends.

Detonation applies the drone's Primary weapon three times over. The target takes that weapon's damage through its warhead, with the drone recorded as the attacker. The drone takes the same damage through the same warhead, with no attacker recorded. A blast of the same strength also goes off at the drone's coordinate, again with no attacker, chaining into whatever it destroys. The flash of light is sized by that same damage figure but is thrown by [`C4Warhead`](/keys/c4warhead/), so it appears only if that warhead sets [`Bright=yes`](/keys/bright/). The weapon's own warhead has no say in it.

Detonation also requires the drone to have been aimed at an object rather than at a bare coordinate. At `0` the measured distance can never fall below it, so the drone never goes off this way. A drone that leaves the playable area still applies its Primary weapon's damage to its target once before it is removed.
