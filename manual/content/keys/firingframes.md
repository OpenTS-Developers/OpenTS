---
key: FiringFrames
summary: The number of frames in one facing's firing animation of a shape-drawn vehicle.
see_also: ["StartFiringFrame", "FiringSyncFrame1", "FiringSyncFrame2", "StandingFrames", "Facings"]
when_omitted:
  kind: value
  value: "0"
---

At `0` the vehicle has no firing animation and its shots are never held back to line up with one. The zero also settles two other defaults in the same section: [`StandingFrames`](/keys/standingframes/) stays at `0`, and [`Facings`](/keys/facings/) drops from 8 to 1 unless the vehicle has a [`Turret=yes`](/keys/turret/). Declaring any count above `0` raises the default standing count to 1 and leaves the default facing count at 8.

The animation is driven by a countdown, not by the step counter that paces walking. Firing starts the countdown at `2 × FiringFrames − 1` and it steps down by one each game frame, so the animation lasts twice the count in game frames, a little under a second at `FiringFrames=6`. The frame drawn is [`StartFiringFrame`](/keys/startfiringframe/), plus the countdown halved, plus the facing block times this count. Halving a falling countdown walks the run from its last frame back to its first, two game frames per frame.

```ini title="art.ini"
[SMECH] ; the Image ID of the stock Wolverine
Voxel=no
WalkFrames=12
FiringFrames=4 ; eight runs of 4, starting at frame 8 × (1 + 12) = 104
```

By default the round leaves first and the countdown starts behind it. [`FiringSyncFrame1`](/keys/firingsyncframe1/) and [`FiringSyncFrame2`](/keys/firingsyncframe2/) move the round into the animation instead.
