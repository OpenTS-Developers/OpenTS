---
title: Give self-healing its own step, interval and ceiling
category: feature
release: 0.2.0
targets:
- type: key
  id: SelfHealStep
  effect: added
- type: key
  id: SelfHealRate
  effect: added
- type: key
  id: SelfHealCap
  effect: added
- type: key
  id: SelfHealingStep
  effect: added
- type: key
  id: SelfHealingRate
  effect: added
- type: key
  id: SelfHealingCap
  effect: added
- type: system
  id: repair
  effect: changed
- type: format
  id: save-games
  effect: changed
credit: [ZivDero, JoyfulShush, Rampastring]
---

`[General] SelfHealStep`, `SelfHealRate` and `SelfHealCap` set how much strength a self-healing object regains, how often it regains it, and the share of its maximum strength it stops at. `SelfHealingStep`, `SelfHealingRate` and `SelfHealingCap` set the same three on one TechnoType. Each falls back to the game-wide value and then to the setting it replaced, so rules that state none of them heal exactly as they did, and a step below one is raised to one, so no value here switches healing off.

A self-healing interval shorter than one frame heals once per frame, where it used to divide by zero and end the game.
