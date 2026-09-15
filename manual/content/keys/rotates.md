---
key: Rotates
summary: Draws the projectile from a set of 32 facing frames instead of always from the same one.
see_also: [Image, AnimLow, AnimHigh, Voxel]
when_omitted:
  kind: value
  value: "no"
---

The frame drawn is picked from the projectile's current heading, rounded to one of 32 directions. Without the setting every projectile of the type is drawn from the first frame of its artwork however it is pointing.

A flight animation overrides it completely. While either [`AnimLow`](/keys/animlow/) or [`AnimHigh`](/keys/animhigh/) is other than `0` the animation frame is drawn and the heading is not read, so the two cannot be combined. A [`Voxel=yes`](/keys/voxel/) projectile is turned in three dimensions rather than picked out of a frame set, and reads neither this setting nor the flight animation.

The setting is read from the art section named by the projectile's [`Image`](/keys/image/). That assignment is required. Without it this setting is never read at all, not even from an art section named after the projectile, which is where its `Voxel` setting is still found.
