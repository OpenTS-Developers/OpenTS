---
title: Make the Comes near waypoint distance configurable
category: feature
release: 0.2.0
targets:
- type: key
  id: ComesNearWaypointDistance
  effect: added
- type: event
  id: TEVENT_NEAR_WAYPOINT
  effect: changed
- type: format
  id: save-games
  effect: changed
credit:
- JoyfulShush
- Rampastring
---

`ComesNearWaypointDistance` in `[General]` sets how near an object must come to a waypoint for the Comes near waypoint trigger event to be satisfied. It is written in leptons; raise it to satisfy the event from farther away, or lower it to require the object closer. The event previously used five cells with no way to change them.

Credits to Rampastring for their original Vinifera implementation for this feature.
