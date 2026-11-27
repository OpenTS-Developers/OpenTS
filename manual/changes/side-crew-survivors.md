---
title: Read survivor and disguise settings from the side a house acts as
category: feature
release: 0.3.0
targets:
- type: key
  id: Crew
  scope: side
  effect: added
- type: key
  id: Technician
  scope: side
  effect: added
- type: key
  id: Engineer
  scope: side
  effect: added
- type: key
  id: Disguise
  scope: side
  effect: added
- type: key
  id: SurvivorDivisor
  scope: side
  effect: added
- type: system
  id: capture
  effect: changed
credit:
- JoyfulShush
- ZivDero
---

A side can now name the types its objects leave as survivors, the type its disguised soldiers show and the divisor its structures use. In the `rules.ini` section named after it, a side sets `Crew`, `Engineer`, `Technician`, `Disguise` and `SurvivorDivisor`. These replace the `[General]` values for the objects of a house that acts as a country on that side; a side that names none keeps the `[General]` value.

Credits to ZivDero for the Vinifera implementation.
