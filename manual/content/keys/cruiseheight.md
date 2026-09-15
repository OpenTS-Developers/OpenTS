---
key: CruiseHeight
summary: The height above ground a jumpjet unit levels off at once it has finished climbing.
see_also: [Climb, WobbleDeviation, FlightLevel]
when_omitted:
  kind: value
  value: "400"
---

The figure is in leptons, with 256 to a cell, and it is the flight level a jumpjet takes off toward and travels at. Two places soften it. A jumpjet closing to within one cell of a destination it has nothing left to shoot at drops to three quarters of the figure. One settling onto its landing spot drops to nothing.

```ini title="rules.ini"
[JumpjetControls]
CruiseHeight=500
```

The same figure decides which layer a jumpjet is drawn in. A unit off the ground but below it is drawn in the air layer, above ground objects; one at or past it is drawn in the top layer, over everything. A hovering or cruising jumpjet drifts above and below its flight level by [`WobbleDeviation`](/keys/wobbledeviation/). A unit holding the full figure has its drift centered on that layer boundary itself rather than sitting to one side of it. Any deviation but zero therefore makes the unit change layers as it bobs.

Over a cell that lies under a bridge, a jumpjet's height is measured from the bridge deck rather than from the ground beneath it. That measure applies to the climb-out, the layer test and the height the locomotor trims toward alike. The flight level such a unit settles at is therefore this figure above the deck, not above the terrain.
