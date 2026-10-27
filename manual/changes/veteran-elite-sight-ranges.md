---
title: Add veteran and elite sight ranges
category: feature
release: 0.2.0
targets:
- type: system
  id: map-visibility
  effect: changed
- type: system
  id: veterancy
  effect: changed
- type: system
  id: target-selection
  effect: changed
- type: key
  id: VeteranSight
  scope: aircrafttype
  effect: added
- type: key
  id: EliteSight
  effect: added
credit:
- JoyfulShush
---

Type sections in `rules.ini` now accept `VeteranSight` and `EliteSight`. A positive `VeteranSight` replaces the type's `Sight=` for its veterans, and a positive `EliteSight` replaces both values for its elites; an elite whose type sets only `VeteranSight` uses it. A promotion makes the object look again, so the new range applies at once.

Aircraft previously revealed terrain from the type's `Sight=` alone. They now use the range for their rank and the `SIGHT` ability multiplier, and include a height bonus from the height where they were placed. An aircraft that appears in flight keeps that bonus while it flies; one that takes off from the ground gains nothing from its flight altitude.

A human-owned object that fights back now measures the attacker against its full sight range, so rank, height and the `SIGHT` ability affect how far it retaliates.
