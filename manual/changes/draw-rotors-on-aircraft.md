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
  id: VoxelRotors
  effect: added
- type: key
  id: Rotor1LandedVoxel
  effect: added
- type: key
  id: Rotor1MovingVoxel
  effect: added
- type: key
  id: Rotor2LandedVoxel
  effect: added
- type: key
  id: Rotor2MovingVoxel
  effect: added
- type: key
  id: Rotor3LandedVoxel
  effect: added
- type: key
  id: Rotor3MovingVoxel
  effect: added
- type: key
  id: Rotor4LandedVoxel
  effect: added
- type: key
  id: Rotor4MovingVoxel
  effect: added
- type: key
  id: CustomRotor
  effect: removed
- type: system
  id: aircraft-rotors
  effect: added

credit: [Orac]
---

Set `VoxelRotors=yes` to use VXL rotor models with separate landed and airborne files. The engine spins each model through a full turn in 32 landed stages or 16 airborne stages without an HVA; a missing or invalid model falls back to the built-in SHP rotor.

SHP mode remains the default and continues to support direction-specific artwork.

The unused `CustomRotor` assignment is no longer read.
