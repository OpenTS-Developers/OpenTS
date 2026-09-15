---
key: HasSpotlight
summary: Whether a structure has a swept spotlight beam.
see_also: [SpotlightLocationRadius, SpotlightMovementRadius, SpotlightRadius, LightIntensity, "system:power"]
when_omitted:
  kind: value
  value: "no"
---

```ini title="rules.ini"
[GASPOT] ; the stock light tower
HasSpotlight=true
```

A structure with the flag is given a beam as it is placed on the map, and the beam is destroyed with it. The beam starts out sweeping, and every second one created starts out sweeping the other way along its arc, so that a row of towers does not swing in unison. The `[General]` spotlight settings cover the rest of its geometry: [`SpotlightLocationRadius`](/keys/spotlightlocationradius/) and [`SpotlightMovementRadius`](/keys/spotlightmovementradius/) place the beam and its pivot, [`SpotlightAngle`](/keys/spotlightangle/), [`SpotlightSpeed`](/keys/spotlightspeed/) and [`SpotlightAcceleration`](/keys/spotlightacceleration/) drive the sweep, and [`SpotlightRadius`](/keys/spotlightradius/) fixes what the beam notices. [Fields, fences and lights](/systems/power/#fields-fences-and-lights) covers the power condition both the drawing and the scan depend on.

A scenario's structure entry sets a [spotlight behavior](/reference/enums/spotlight-behavior/) of its own, so a map may place one of these towers with its beam circling, following a target, or switched off and invisible instead of sweeping. The [Change spotlight behavior](/mapping/actions/taction-change-spotlight-behavior/) trigger action moves every tagged structure that has a beam between those states mid-scenario. A structure without the flag has no beam for either of them to reach.

The flag has nothing to do with the colored glow a structure casts on the ground around it, which [`LightIntensity`](/keys/lightintensity/) covers; a type may set either, both, or neither.
