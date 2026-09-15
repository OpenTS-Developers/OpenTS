---
key: Rate
scope: animtype
label: Playback rate
when_omitted:
  kind: value
  value: "900"
  note: The stored wait falls back to the constructor default of one game frame, so the animation advances every frame.
---

The value is the animation's playback rate in frames of the animation per minute of game time. The engine stores `900` divided by it, in whole numbers, as the wait between one animation frame and the next. `Rate=900` advances every game frame, and `Rate=450` every second one. A value of `0` stores no wait at all, and neither does any negative value except `-1`, which counts as absent.
