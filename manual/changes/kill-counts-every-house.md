---
title: Count kills of every house on maps with more than 20 houses
category: fix
release: 0.3.0
targets:
- type: event
  id: TEVENT_NUNITS_DESTROYED
  effect: changed
- type: event
  id: TEVENT_NBUILDINGS_DESTROYED
  effect: changed
credit:
- ZivDero
---

On a map with more than 20 houses, a house that destroyed or captured an object of a house beyond the first 20 could have its own loss counts raised, which [Destroyed, Units, #...](/mapping/events/tevent-nunits-destroyed/) and [Destroyed, Buildings, #...](/mapping/events/tevent-nbuildings-destroyed/) read. Other values of the same house, such as the center of its base, could be overwritten too. Such kills are now recorded like any other.
