---
key: AnimRate
summary: The number of game frames each frame of the projectile's flight animation is held for.
see_also: [AnimLow, AnimHigh, Image]
when_omitted:
  kind: value
  value: "0"
---

A counter is loaded with the figure when the projectile is created and reloaded every time the shape steps, counting down one per game frame; the shape advances when it reaches zero. `AnimRate=1` therefore shows a new frame every game frame, 15 a second, and `AnimRate=4` shows one every four.

The figure is read only while the animation runs, which needs [`AnimLow`](/keys/animlow/) or [`AnimHigh`](/keys/animhigh/) to be other than `0`.

:::caution[`AnimRate=0` does not hold the animation still]
The counter is a single byte, and counting down from zero wraps it to 255 rather than stopping it. The shape steps once every 256 game frames (a little over seventeen seconds) instead. Figures above `255` wrap the same way.
:::

The setting is read from the art section named by the projectile's [`Image`](/keys/image/). That assignment is required. Without it the lookup runs against an empty section name, so nothing here is read, not even from an art section named after the projectile, which is where its [`Voxel`](/keys/voxel/) setting is still found.
