---
key: HasRadialIndicator
scope: aircrafttype
label: Radius ring
when_omitted:
  kind: value
  value: "no"
---

Only a structure draws anything, and only a [`CloakGenerator=yes`](/keys/cloakgenerator/) or [`SensorArray=yes`](/keys/sensorarray/) one. Everything else in this scope stores the value and never uses it.

The ring is drawn while such a structure is selected, switched on, and owned by a player-controlled house. It is an ellipse sized from [`CloakRadiusInCells`](/keys/cloakradiusincells/), drawn in [`RadialColor`](/keys/radialcolor/) with four spokes sweeping round it. Its geometry is fixed in the engine; the only part of it a type can change is its color and how far out it reaches.
