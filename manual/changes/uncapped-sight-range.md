---
title: Let a sight range above ten cells take effect
category: feature
release: 0.3.0
targets:
- type: system
  id: map-visibility
  effect: changed
- type: key
  id: Sight
  effect: changed
- type: key
  id: AttackingAircraftSightRange
  effect: changed
- type: key
  id: AircraftFogReveal
  effect: changed
- type: key
  id: RevealTriggerRadius
  effect: changed
- type: key
  id: DropZoneRadius
  effect: changed
credit:
- JoyfulShush
---

`Sight=` in a type's `rules.ini` section now takes effect at any size. Before, every look and reveal stopped at ten cells, so a larger range, height bonus, veteran bonus or reveal radius had no further effect. The separate reveal radii for aircraft fire, aircraft fog, drop zones and the reveal-around-waypoint action stopped at the same cap and now also take effect.
