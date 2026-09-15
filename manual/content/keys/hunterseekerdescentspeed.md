---
key: HunterSeekerDescentSpeed
summary: Leptons a hunter seeker drops each frame.
see_also: [HunterSeekerAscentSpeed, HunterSeekerEmergeSpeed, HunterSeekerDescendProximity, HunterSeeker, "system:superweapons"]
when_omitted:
  kind: value
  value: "0"
---

While an airborne [hunter seeker](/systems/superweapons/#hunter-seeker) is above its flight level, it drops each frame by the smaller of this figure and its height above the ground. That second limit is the drone's whole altitude rather than the gap down to the level it is heading for. A figure larger than the gap therefore takes it past that level in one step, and leaves the climb to put it back. The drone re-levels once the climb has restored that level. The stock rules use `50`, about a fifth of a cell a frame. A drone reduced to no strength is pulled down by an accelerating fall of its own, on top of whatever this figure takes off. The fall adds one lepton on the first frame, two on the next, and so on. It is destroyed the moment that fall takes it to the ground, throwing a fixed blast of 1000 damage through [`C4Warhead`](/keys/c4warhead/) at its landing point.

An ordinary aircraft stages its descent, taking a step of a twentieth of the remaining gap and holding that step between 20 and 50 leptons a frame. A hunter seeker never uses that staged descent. [`HunterSeekerDescendProximity`](/keys/hunterseekerdescendproximity/) covers where the drone is told to descend to.

At `0` the step is zero and the drone holds whatever altitude it has reached.
