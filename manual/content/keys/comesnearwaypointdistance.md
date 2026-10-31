---
key: ComesNearWaypointDistance
summary: Distance in leptons from a waypoint at which the Comes near waypoint trigger event is satisfied.
when_omitted:
  kind: value
  value: "1280"
  note: Five cells.
---

`ComesNearWaypointDistance` sets how near the tagged object must come to the named waypoint for the [Comes near waypoint](/mapping/events/tevent-near-waypoint/) trigger event to be satisfied. An object closer to the waypoint than this distance satisfies it; one that is farther away does not. Raising the value satisfies the event from farther away, and lowering it requires the object to come closer.

```ini title="rules.ini"
[General]
ComesNearWaypointDistance=2560  ; the event is satisfied within ten cells of the waypoint
```

Write the distance in [leptons](/glossary/#lepton), the engine's unit of distance. One cell is 256 leptons, so `512` reaches two cells. A value written as a cell count is read as that many leptons: `5` reaches five leptons, not five cells.

With a value of `0`, only an object whose center coincides with the waypoint's center satisfies the event. Any value below `0` leaves the event unsatisfied for every object.
