---
key: WalkFrames
summary: The number of frames in one facing's walking animation of a shape-drawn vehicle.
see_also: ["StartWalkFrame", "Facings", "StandingFrames", "WalkRate", "Turret"]
when_omitted:
  kind: value
  value: "12"
---

The facings' walk blocks sit end to end, so the figure doubles as the distance from one block to the next. A moving vehicle is drawn at [`StartWalkFrame`](/keys/startwalkframe/) plus its facing block times this count, plus its step count taken as a remainder against this count. The [`WalkRate`](/keys/walkrate/) setting paces that step count. A vehicle standing still in a cell it occupies, with no [`StandingFrames`](/keys/standingframes/) of its own, is drawn from the same block on its first frame. The occupation check reads a flag the locomotor maintains: it clears while the vehicle is under way and returns when the vehicle settles. A still vehicle with the flag out is drawn from the file's first [`Facings`](/keys/facings/) frames instead.

```ini title="art.ini"
[MMCH] ; the Image ID of the stock Titan
Voxel=no
WalkFrames=15 ; eight blocks of 15, frames 0-119
```

A [`Turret=yes`](/keys/turret/) vehicle drawn from shape artwork takes its [`TurretFacings`](/keys/turretfacings/) turret frames from `8 × WalkFrames` onward (frame 120 in the fragment above). That eight is fixed: the base is `8 × WalkFrames` whatever the vehicle's Facings is. A sixteen-facing walker fills sixteen walk blocks, so the default base lands inside the ninth block and the engine draws the turret from walk data. Such a vehicle has to name [`StartTurretFrame`](/keys/startturretframe/) to put the strip after the sixteenth block, at frame `240` with the fragment's `WalkFrames=15`.

:::danger[A count of zero divides by zero]
The walk frame is worked out with a remainder against this count, without a guard, every time a moving shape-drawn vehicle is drawn. `WalkFrames=0` therefore brings the game down as soon as such a vehicle moves within view. The count is kept in a single signed byte. `WalkFrames=256` stores that same zero, and any figure above 127 stores as a negative number, so the facing blocks run backwards from `StartWalkFrame` instead of after it.
:::
