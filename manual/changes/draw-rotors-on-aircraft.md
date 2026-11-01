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

credit: [Orac]
---

When `Rotors` is set to yes in an aircraft's art.ini entry, the game will now draw a rotor effect over the unit, following the Red Alert convention for this.

As in Red Alert, a rotor is a 12 frame shape. The game cycles through the first 4 frames while flying, and through the second 8 frames while landed. The rotor is drawn at `Translucency=25` when landed, and `Translucency=50` when flying.

The `RotorCount` key has been added. When ommitted, the game assumes 1 rotor, but it supports up to 4.

By default, the game will look for the files `LROTOR.SHP` and `RROTOR.SHP` and use them if present. If the aircraft has more than one rotor, `LROTOR.SHP` will be assigned to rotors 1 and 3, `RROTOR.SHP` will be assigned to rotors 2 and 4.

To load custom rotor shapes, the keys `Rotor1Anim`, `Rotor2Anim`, `Rotor3Anim`, and `Rotor4Anim` have been added. These accept shape files set up as above, with 4 flying frames followed by 8 landed frames. Note that not all `Rotor#Anim` keys need to be specified, if an aircraft has only `Rotor1Anim` with a `RotorCount` greater than 1, `Rotor1Anim` will be used for all rotors. If an aircraft has more than one rotor and both `Rotor1Anim` and `Rotor2Anim`, `Rotor1Anim` will be used for rotors 1 and 3, `Rotor2Anim` will be used for rotors 2 and 4. This behaviour can be overriden by specifying more rotor anims.

Rotor positions can be set by FLH (Forward, Lateral, Height) value, using `Rotor1FLH`, `Rotor2FLH`, `Rotor3FLH`, and `Rotor4FLH`.

While moving, many helicopters tilt forward. To visually support this, `Rotor1TiltedAnims`, `Rotor2TiltedAnims`, `Rotor3TiltedAnims` and `Rotor4TiltedAnims` have been added. Each of these expects a list of 8 rotor shapes in clockwise order starting at North (facing towards the top right corner of the screen). The game will select the rotor anim closest to the unit's direction of travel and display this instead of the flat rotor anim while moving in a direction. Each of these tilted rotor shapes follows the same rules as the normal rotors and uses the same FLH.
