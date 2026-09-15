---
key: RadarCombatFlashTime
summary: Frames a damaged object's radar blip keeps flashing for.
see_also: ["system:map-visibility", FlashFrameTime]
when_omitted:
  kind: value
  value: "21"
---

Any damage that registers a result reloads the object's flash timer with this many frames. Fresh damage restarts the count rather than extending it. The timer is set on every object that takes damage, but only an object of the local player's own house is repainted from it. The value therefore governs how long the player's own damaged objects stay marked on the radar.

While the timer runs the blip is repainted every [`FlashFrameTime`](/keys/flashframetime/) frames. At the default the timer covers three such repaints.
