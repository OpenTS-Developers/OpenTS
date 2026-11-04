---
key: AircraftFogReveal
summary: Radius in cells that an airborne aircraft with no sight of its own clears of fog.
see_also: ["system:map-visibility", Sight, VeteranSight, EliteSight]
when_omitted:
  kind: value
  value: "6"
---

Only an airborne aircraft with no sight range uses this radius, and only while [fog of war](/keys/fogofwar/) is on; without fog, such an aircraft reveals nothing while airborne. Other aircraft reveal terrain with their sight range, and a landed aircraft sees 1 cell.

A positive [`VeteranSight`](/keys/veteransight/#scope-aircrafttype) or [`EliteSight`](/keys/elitesight/) for the aircraft's rank gives it a range even when its type sets [`Sight=0`](/keys/sight/), so it stops using this radius.

Within the radius, the aircraft lifts fog only from cells that are already out from under the shroud. It never uncovers shrouded cells, so it clears the view along its path without mapping new ground. A radius above 10 acts as 10.

High ground can block cells from this reveal while [`RevealByHeight=yes`](/keys/revealbyheight/) and the aircraft flies below half the `[General]` [`FlightLevel`](/keys/flightlevel/#scope-global-rules). At or above that height, high ground blocks nothing, and the aircraft lifts fog from every cell in the radius that is already out from under the shroud. The `FlightLevel` set on the aircraft type does not move this threshold.
