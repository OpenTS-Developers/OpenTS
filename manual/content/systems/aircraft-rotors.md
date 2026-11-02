---
title: Aircraft rotors
summary: How rotor count, artwork, direction, placement, and visibility affect aircraft drawing.
category: rendering-presentation
keys:
  - Rotors
  - RotorCount
  - Rotor1Anim
  - Rotor2Anim
  - Rotor3Anim
  - Rotor4Anim
  - Rotor1FLH
  - Rotor2FLH
  - Rotor3FLH
  - Rotor4FLH
  - Rotor1TiltedAnims
  - Rotor2TiltedAnims
  - Rotor3TiltedAnims
  - Rotor4TiltedAnims
---

OpenTS draws separate rotor shapes only when an aircraft's `art.ini` image section sets [`Rotors=yes`](/keys/rotors/). The aircraft must have positive strength and be visible to the current player; a hidden aircraft's rotors are not drawn.

Rotor shapes are 25% translucent when the aircraft is landed and 50% when it is airborne. A cloaked aircraft uses 50% and 75% translucency, respectively.

## Rotor count and frame selection

[`RotorCount`](/keys/rotorcount/) sets how many rotors the engine draws, up to four. A value of zero or less draws none. Each rotor uses a 12-frame SHP: frames 0–3 while airborne and frames 4–11 while landed.

If custom artwork is unavailable, rotor 1 and rotor 3 use `LROTOR.SHP`, while rotor 2 and rotor 4 use `RROTOR.SHP`. A rotor is not drawn if neither its custom SHP nor its built-in SHP loads.

## Custom artwork

[`Rotor1Anim`](/keys/rotor1anim/), [`Rotor2Anim`](/keys/rotor2anim/), [`Rotor3Anim`](/keys/rotor3anim/), and [`Rotor4Anim`](/keys/rotor4anim/) name custom SHP files by their base names; the engine adds `.SHP`. Keep each name to 15 characters. Each custom SHP must have the same 12-frame layout as the built-in rotor shapes.

The engine counts consecutive `Rotor#Anim` names starting at `Rotor1Anim` and stops at the first missing name. It repeats that sequence for additional rotors: one custom name serves every rotor, two names alternate, and longer sequences repeat in order. A gap causes later `Rotor#Anim` settings to be ignored. If a custom SHP cannot be loaded, the engine uses that rotor's built-in SHP instead.

[`Rotor1FLH`](/keys/rotor1flh/), [`Rotor2FLH`](/keys/rotor2flh/), [`Rotor3FLH`](/keys/rotor3flh/), and [`Rotor4FLH`](/keys/rotor4flh/) move their rotors along the aircraft's local forward, lateral, and height axes. Flat and direction-specific artwork use the same offset.

```ini title="art.ini"
[MYHELI]
Rotors=yes
RotorCount=2
Rotor1Anim=MYROTOR
Rotor1FLH=32,0,48
Rotor2FLH=-32,0,48
Rotor1TiltedAnims=MYROT_N,MYROT_NE,MYROT_E,MYROT_SE,MYROT_S,MYROT_SW,MYROT_W,MYROT_NW
```

In this example, both rotors use `MYROTOR`; `Rotor1TiltedAnims` supplies the eight directional versions for both.

## Directional artwork

Each of [`Rotor1TiltedAnims`](/keys/rotor1tiltedanims/), [`Rotor2TiltedAnims`](/keys/rotor2tiltedanims/), [`Rotor3TiltedAnims`](/keys/rotor3tiltedanims/), and [`Rotor4TiltedAnims`](/keys/rotor4tiltedanims/) must supply eight comma-separated SHP base names, ordered from north (facing towards the top right corner of the screen) clockwise. Each filename in the list may contain at most 15 characters; an empty position leaves that facing without a directional shape.

While the aircraft is moving, the engine selects the directional shape for its current facing. If that shape is missing, it uses the corresponding flat custom shape, then the built-in shape. Directional shapes are considered only for the consecutive custom-art sequence described above; setting a `Rotor#TiltedAnims` key without a matching `Rotor#Anim` does not enable custom rotor artwork.
