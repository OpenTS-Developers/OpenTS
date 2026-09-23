---
title: Keep units that cannot enter the shroud from rallying into it
category: fix
release: 0.2.0
targets:
- type: system
  id: production
  effect: changed
- type: key
  id: MoveToShroud
  effect: changed
- type: key
  id: AllowShroudedSubteranneanMoves
  effect: changed
credit: [ZivDero, dkeeton]
---

A player's object leaving a factory no longer follows a rally point that is still shrouded for the player if its type cannot be ordered into the shroud. That covers a type whose `rules.ini` section has `MoveToShroud=no`, the default for aircraft types, and a subterranean type while `rules.ini` sets `AllowShroudedSubteranneanMoves=no` under `[General]`. Such an object leaves as if the factory had no rally point, and uses the rally point again once the player uncovers that ground.

dkeeton is credited for the ts-patches patch this follows, which refused such a rally point at the cursor instead and exempted every barracks.
