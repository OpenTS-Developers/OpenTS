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

Two settings now decide who sees through a disguise. A type set `DetectDisguise=yes` scores a
disguised soldier like any other candidate, and `AIDetectDisguise=yes` in `[AI]` gives every
computer-controlled house the same sight while leaving a player's own units passing the
soldier over.

The rejection used to read the candidate's type alone, with no way for the rules to grant an
exception — not even to the dog, which the engine's own source comments described as exempt.
