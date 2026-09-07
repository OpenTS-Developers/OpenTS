---
key: Facings
summary: The number of facings a shape-drawn vehicle's artwork is cut into.
see_also: ["WalkFrames", "StandingFrames", "FiringFrames", "TurretFacings", "RotCount", "Voxel"]
when_omitted:
  kind: computed
  note: 8, or 1 for a vehicle that declares no firing frames and no turret.
---

Only a vehicle drawn from shape artwork reads this figure; a [`Voxel=yes`](/keys/voxel/) vehicle is turned rather than picked out of a set of frames.

The setting does two separate jobs. It is the number of facings the vehicle is drawn from, and it is the multiplier the engine uses when it works out where the standing, firing and death blocks begin.

Per-facing drawing happens at `8`, `16`, `32` and `64`. The facing drawn is the vehicle's own heading rounded to that many compass points and then advanced by an eighth of a turn, so northwest is facing 0 at every count: at `8` a vehicle pointing northwest is drawn at facing 0, one pointing north at facing 1, and so on round to west at facing 7. At any other value the facing is fixed at 0, so every instance is drawn the same way whichever way it points.

```ini title="art.ini"
[JUGGER] ; the Image ID of the stock Juggernaut
Voxel=no
WalkFrames=15  ; blocks of 15 frames, one per facing, from frame 0
StandingFrames=0
Facings=8
```

The same figure is the stride in every frame number the engine derives rather than reads: [`StartStandFrame`](/keys/startstandframe/), [`StartFiringFrame`](/keys/startfiringframe/) and [`StartDeathFrame`](/keys/startdeathframe/) each multiply a per-facing frame count by it. Raising it therefore pushes those defaults further up the file even though it adds no facings to the drawing, and lowering it to `1` collapses them onto the walk block.

:::caution[The turret strip does not move with this figure]
A [`Turret=yes`](/keys/turret/) vehicle takes its turret strip from `8 × WalkFrames` whatever this figure holds. Artwork cut into more facings than that has to start its walk block after the strip, or name [`StartTurretFrame`](/keys/startturretframe/).
:::
