---
key: FlashFrameTime
summary: Frames between repaints of a flashing radar blip.
see_also: ["system:map-visibility", RadarCombatFlashTime]
when_omitted:
  kind: value
  value: "7"
---

The value is used twice while a damaged object's [`RadarCombatFlashTime`](/keys/radarcombatflashtime/) timer runs. The blip is repainted each time the frames remaining are an exact multiple of it, and the color plotted alternates between the house color and its inverse from one block of this many frames to the next. The value therefore sets the beat of the flash rather than its length.

Both halves are gated on the object belonging to the local player's own house, so an enemy object takes damage without its blip changing at all. `FlashFrameTime=0` is used as a divisor with no guard.
