---
key: StandingFrames
summary: The number of frames in one facing's standing artwork of a shape-drawn vehicle.
see_also: ["StartStandFrame", "WalkFrames", "FiringFrames", "Facings"]
when_omitted:
  kind: computed
  note: 1 for a vehicle whose FiringFrames is above 0, and 0 otherwise.
---
At `0` the vehicle has no standing artwork of its own and is drawn from the first frame of its facing's walk run instead. Above `0` a vehicle at rest in a cell it occupies is drawn at [`StartStandFrame`](/keys/startstandframe/) plus its facing block times this count. The count is also the stride the engine assumes between one facing's standing run and the next. The count is kept in a single signed byte, so `256` stores as zero and any figure above `127` stores as a negative number.

On a [`Turret=yes`](/keys/turret/) vehicle with firing frames the default eight facings and standing count put the derived [`StartStandFrame`](/keys/startstandframe/) at `8 × WalkFrames`, the exact frame the turret strip starts at. The engine moves neither block: a resting body is drawn straight out of the strip's frames with the turret layered on top. Artwork that wants a distinct standing frame writes the value out.

```ini title="art.ini"
[REAPER] ; the Image ID of the stock Cyborg Reaper, which has no turret, so no strip competes for these frames
Facings=8
StandingFrames=1
StartStandFrame=0 ; frames 0-7
WalkFrames=12
StartWalkFrame=8  ; the walk block follows the standing strip, frames 8-103
```

:::caution[The standing artwork never animates]
Only the first frame of a facing's standing run is ever drawn. Nothing advances a resting vehicle through the run, so a count above `1` reserves frames that the vehicle never shows and pushes the derived [`StartFiringFrame`](/keys/startfiringframe/) further up the file for nothing.
:::
