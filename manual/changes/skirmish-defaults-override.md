---
title: Override skirmish defaults from SUN.INI
category: feature
release: 0.2.0
targets:
- type: key
  id: OverrideSkirmishDefaults
  effect: added
- type: key
  id: SkirmishDefaultAILevel
  effect: added
- type: key
  id: SkirmishDefaultAIPlayers
  effect: added
- type: key
  id: SkirmishDefaultBases
  effect: added
- type: key
  id: SkirmishDefaultBridges
  effect: added
- type: key
  id: SkirmishDefaultCrates
  effect: added
- type: key
  id: SkirmishDefaultCredits
  effect: added
- type: key
  id: SkirmishDefaultFog
  effect: added
- type: key
  id: SkirmishDefaultGameSpeed
  effect: added
- type: key
  id: SkirmishDefaultMultiEngineer
  effect: added
- type: key
  id: SkirmishDefaultRedeploy
  effect: added
- type: key
  id: SkirmishDefaultShortGame
  effect: added
- type: key
  id: SkirmishDefaultTechLevel
  effect: added
- type: key
  id: SkirmishDefaultUnitCount
  effect: added
credit:
- fjlaubscher
---

`OverrideSkirmishDefaults=yes` under `[siliconsun]` in `SUN.INI` starts a skirmish with nothing saved to restore from the `SkirmishDefault*` values in the same section instead of the engine's built-in defaults. The values cover the credits, unit count, tech level, game speed, AI count and difficulty, and each game-options toggle. A saved setup still wins when `PersistSkirmishSettings=yes`.
