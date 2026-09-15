---
key: AnimLow
summary: The frame the projectile's flight animation returns to when it loops.
see_also: [AnimHigh, AnimRate, Rotates, Image]
when_omitted:
  kind: value
  value: "0"
---

Once the shape steps past [`AnimHigh`](/keys/animhigh/) it starts again here rather than at the beginning of the artwork, which is how a projectile holds a few leading frames outside the loop.

The animation runs only while one of the two frame numbers is other than `0`. With both left at `0` the shape is picked from the projectile's heading instead, which is what [`Rotates`](/keys/rotates/) controls. With either set, the animation frame is drawn and the heading is never read.

A new projectile always starts on frame `0`, so a value above zero is passed over on the first run through and only takes effect from the second. Setting it higher than `AnimHigh` leaves the shape standing still on this frame, because the very next step is already past the last frame of the loop.

The figure is stored in a single byte, so `256` stores as zero and anything above `255` wraps around.

The setting is read from the art section named by the projectile's [`Image`](/keys/image/). That assignment is required. Without it the lookup runs against an empty section name, so nothing here is read, not even from an art section named after the projectile, which is where its [`Voxel`](/keys/voxel/) setting is still found.
