---
title: Draw shape vehicles cut into more than eight facings
category: feature
release: 0.2.0
targets:
- type: key
  id: Facings
  effect: changed
- type: key
  id: TurretFacings
  effect: added
- type: key
  id: StartTurretFrame
  effect: added
- type: key
  id: Anim
  effect: changed
credit: [ZivDero, CCHyper]
---

Artwork that declared `Facings` above eight was drawn at a single facing. A shape-drawn vehicle is now drawn per facing at `16`, `32` and `64` as well as `8`, and a weapon's `Anim` list selects by facing at those lengths too. Any other count still draws one facing, and artwork written for `8` is drawn exactly as before.

A turret takes its own count from the new `TurretFacings` and its strip from the new `StartTurretFrame`. Both default to what the engine already drew, so no existing artwork moves.
