---
title: Draw rotors on aircraft
category: feature
release: 0.2.0
targets:
- type: key
  id: Rotors
  effect: changed
- type: key
  id: RotorCount
  effect: added
- type: key
  id: Rotor1Anim
  effect: added
- type: key
  id: Rotor2Anim
  effect: added
- type: key
  id: Rotor3Anim
  effect: added
- type: key
  id: Rotor4Anim
  effect: added
- type: key
  id: Rotor1TiltedAnims
  effect: added
- type: key
  id: Rotor2TiltedAnims
  effect: added
- type: key
  id: Rotor3TiltedAnims
  effect: added
- type: key
  id: Rotor4TiltedAnims
  effect: added
- type: key
  id: Rotor1FLH
  effect: added
- type: key
  id: Rotor2FLH
  effect: added
- type: key
  id: Rotor3FLH
  effect: added
- type: key
  id: Rotor4FLH
  effect: added
- type: key
  id: CustomRotor
  effect: removed
- type: system
  id: aircraft-rotors
  effect: added

credit: [Orac]
---

Set `Rotors=yes` in an aircraft's `art.ini` image section to draw separate rotor shapes. Each shape uses 12 frames: 0–3 while airborne and 4–11 while landed.

`RotorCount` sets how many rotors are drawn, up to four. `Rotor#Anim` and `Rotor#FLH` select custom shapes and their positions; without a loaded custom shape, the engine uses `LROTOR.SHP` and `RROTOR.SHP` when available.

`Rotor#TiltedAnims` selects facing-specific shapes while the aircraft moves.

The unused `CustomRotor` assignment is no longer read.
