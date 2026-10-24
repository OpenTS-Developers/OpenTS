---
title: Add feedback to players after a promotion
category: feature
release: 0.2.0
targets:
- type: system
  id: veterancy
  effect: changed
- type: key
  id: UpgradeVeteranSound
  effect: added
- type: key
  id: UpgradeEliteSound
  effect: added
- type: key
  id: VoxUnitPromoted
  effect: added
- type: key
  id: VeteranFlashTimer
  effect: added
- type: key
  id: EliteFlashTimer
  effect: added
credit:
- JoyfulShush
---

Announced promotions can now play a sound, request an EVA line, and flash the promoted object, but only for the local player's houses; another house's promotion adds none. `UpgradeVeteranSound` and `VeteranFlashTimer` under `[AudioVisual]` in `rules.ini` cover a rise to veteran, and `UpgradeEliteSound` and `EliteFlashTimer` cover a rise to elite, including one that skips the veteran rank; `VoxUnitPromoted` names the EVA line for either. With all five keys unset, a promotion makes no sound and does not flash.

The feedback covers promotions from combat, a veterancy crate, and an armory. Promotions from `InitialVeteran`, a reinforcement's `VeteranLevel`, Drop Pods, the Make Elite trigger action, and experience set by a map's placed-object record stay silent, as does a rise from below rookie to rookie.

A promotion also makes the object look again, so a sight bonus from the new rank reveals the larger area at once.
