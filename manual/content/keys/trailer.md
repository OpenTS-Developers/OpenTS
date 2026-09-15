---
key: Trailer
summary: The animation left behind the projectile as it flies.
see_also: [Image, Inviso]
when_omitted:
  kind: value
  value: none
---

One copy of the animation is placed at the projectile's current position every third game frame, five times a second. The count runs on the game clock rather than on the projectile's own age, so every trailing projectile in a match drops its puffs on the same frames. Each copy waits one frame and then plays through once.

The trail comes from the flight logic rather than the drawing, so a [`Voxel=yes`](/keys/voxel/) projectile leaves one exactly as a shape-drawn projectile does. An [`Inviso=yes`](/keys/inviso/) projectile is already at its target before its first logic frame, so it leaves at most one puff and that puff appears on the target.

The setting is read from the art section named by the projectile's [`Image`](/keys/image/). That assignment is required. Without it this setting is never read at all, not even from an art section named after the projectile, which is where its `Voxel` setting is still found.

```ini title="art.ini"
[MYMISSILEART] ; the Image ID that the projectile's Image assignment names
Trailer=SMOKEY2 ; an AnimType registered in [Animations]
```
