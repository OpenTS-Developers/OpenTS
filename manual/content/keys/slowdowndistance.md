---
key: SlowdownDistance
summary: How far short of its destination an object begins slowing down, in leptons.
see_also: ["Accelerates", "DeaccelerationFactor", "IsDropship"]
when_omitted:
  kind: value
  value: "500"
---

The figure is a raw lepton count, not a number of cells: a cell is 256 leptons across, so the engine's own 500 is a little under two cells and the dropship's 2000 is nearly eight.

```ini title="rules.ini"
[MYDROP] ; an AircraftType registered in [AircraftTypes]
SlowdownDistance=2000
```

A driving vehicle with [`Accelerates=yes`](/keys/accelerates/) begins braking once it is within this distance of its destination. An aircraft uses it differently. The fly locomotor sets the aircraft's target throttle to the remaining distance divided by this figure, so the approach tapers off smoothly over the whole stretch rather than braking at a fixed rate. Inside a tenth of the figure, the aircraft drops to a crawl and then to a stop.

An [`IsDropship=yes`](/keys/isdropship/) type reads it three times more on the way in. It starts tipping its nose over the moment it comes within the figure. The nose reaches its full [`PitchAngle`](/keys/pitchangle/) four tenths of the way in and holds that angle from there. Its cruising height is meant to ease down to a third of its [`FlightLevel`](/keys/flightlevel/#scope-aircrafttype) over the same stretch, but the blend is worked out in whole numbers and always comes out at the end of its range. The height drops to a third the instant the dropship crosses this distance, and stays there.
