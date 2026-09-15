---
key: Normalized
scope: aircrafttype
label: Building animation rate
when_omitted:
  kind: value
  value: "no"
---

Only a building reads the flag. When this flag is yes, the first animation state a building takes, and any state set while the scenario is being set up, does not use its configured frame delay as written. The delay is rescaled against the game speed setting, so the animation keeps roughly the same rate on the clock however fast or slow the match is running.

The flag applies only on the path that reads it. Two other paths rescale the construction and idle states unconditionally, whatever this flag says: the queued change of animation state, and the restart at the end of a loop.

An aircraft, infantry type, or vehicle stores the flag and never reads it. No art setting gives an aircraft the same control: its body is drawn turned to the aircraft's facing in every frame.
