---
title: Make seeing through a disguise a stated ability
category: feature
release: 0.1.0
targets:
- type: key
  id: DetectDisguise
  effect: added
- type: key
  id: AIDetectDisguise
  effect: added
- type: key
  id: Disguised
  effect: changed
- type: system
  id: target-selection
  effect: changed
credit: [ZivDero]
---

Two settings now decide who sees through a disguise. A type set `DetectDisguise=yes` scores a disguised soldier like any other candidate. `AIDetectDisguise=yes` in `[AI]` gives every computer-controlled house the same sight, while a player's own units still pass the soldier over.

The rejection used to read the candidate's type alone, with no way for the rules to grant an exception. Not even the dog was exempt, though the engine's own source comments described it as such.
