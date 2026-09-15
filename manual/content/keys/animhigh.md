---
key: AnimHigh
summary: The last frame of the projectile's looping flight animation.
see_also: [AnimLow, AnimRate, Rotates, Image]
when_omitted:
  kind: value
  value: "0"
---

The projectile's shape steps forward one frame at a time and returns to [`AnimLow`](/keys/animlow/) once it passes this one. The pair fixes the loop and [`AnimRate`](/keys/animrate/) fixes how fast it runs.

The animation runs only while one of the two frame numbers is other than `0`. With both left at `0` the shape is picked from the projectile's heading instead, which is what [`Rotates`](/keys/rotates/) controls. With either set, the animation frame is drawn and the heading is never read.

A new projectile always starts on frame `0` rather than on `AnimLow`, so its first pass runs from the beginning of the artwork and only later passes are confined to the loop.

```ini title="art.ini"
[MYFLAREART] ; the Image ID that the projectile's Image assignment names
AnimPalette=yes
AnimLow=0
AnimHigh=5 ; six frames, restarting at the first
AnimRate=1
```

The figure is stored in a single byte, so `256` stores as zero and anything above `255` wraps around.

The setting is read from the art section named by the projectile's [`Image`](/keys/image/). That assignment is required. Without it the lookup runs against an empty section name, so nothing here is read, not even from an art section named after the projectile, which is where its [`Voxel`](/keys/voxel/) setting is still found.
