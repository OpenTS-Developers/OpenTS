---
key: FlightLevel
scope: global-rules
label: Rules-wide cruising altitude
see_also: ["AircraftFogReveal"]
when_omitted:
  kind: value
  value: "500"
---

The figure is a height above the ground in leptons. Any type that does not set a [cruising altitude of its own](/keys/flightlevel/#scope-aircrafttype) takes this one. The shipped value is `600`, the height every stock aircraft flies at, bar the dropship and the two hunter seekers, which set figures of their own.

Three other paths read it directly rather than through a type. An animation that is not a ground-layer animation starts at this height. Every object below six tenths of it is drawn in the ground layer, and everything at or above it is drawn in the top layer. That is what decides whether an object passes over or under the things around it. An aircraft below half of it takes the height test that blocks [`AircraftFogReveal`](/keys/aircraftfogreveal/) behind high ground, which [who looks, and when](/systems/map-visibility/#who-looks-and-when) covers.
