---
title: Let a type set its own cloak and uncloak sounds
category: feature
release: 0.2.0
targets:
- type: key
  id: CloakSound
  scope: aircrafttype
  effect: added
- type: key
  id: UncloakSound
  effect: added
- type: system
  id: cloaking
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: 
- JoyfulShush
- CCHyper
---

`CloakSound=` in a vehicle, infantryman, aircraft or structure type's `rules.ini` section sets the sound that type plays at its position as it starts to hide. It replaces the `[AudioVisual]` `CloakSound` for that type when the key is set.

`UncloakSound=` in the same section sets the sound the type plays as it starts to reappear. A type that sets only `CloakSound=` still plays the `[AudioVisual]` `CloakSound` when it reappears, since no rules-wide uncloak sound exists.

Credits to CCHyper for their original Vinifera implementation for this feature.
