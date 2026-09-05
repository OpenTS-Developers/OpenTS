---
title: Define sounds with the Yuri's Revenge SOUND.INI keys
category: feature
release: 0.2.0
breaking: true
migration:
- A `Volume=` above 1 is now a percentage, so a value that used to boost a sound beyond its sample now quietens it. Remove such values or write `Volume=100`.
targets:
- type: format
  id: sound-ini
  effect: changed
- type: key
  id: Volume
  effect: changed
- type: key
  id: Priority
  effect: changed
- type: key
  id: Range
  effect: added
  scope: sounds
- type: key
  id: Type
  effect: added
  scope: sounds
- type: key
  id: Sounds
  effect: added
- type: key
  id: MinVolume
  effect: added
- type: key
  id: Limit
  effect: added
- type: key
  id: Loop
  effect: added
- type: key
  id: LoopLimit
  effect: added
- type: key
  id: Delay
  effect: added
- type: key
  id: FShift
  effect: added
- type: key
  id: VShift
  effect: added
- type: key
  id: Control
  effect: added
- type: key
  id: Attack
  effect: added
- type: key
  id: Decay
  effect: added
- type: key
  id: Channels
  effect: added
credit: [ZivDero]
---

A sound section may now list several samples, loop them with attack and decay samples and random or ordered bodies, delay between cycles, shift pitch and volume at random, limit how many copies play at once, and set how far from the view it is heard. `[Defaults]` supplies the values a section omits and `[General] Channels=` sets how many sound effects play at once. The shipped files read unchanged, and sections written for Yuri's Revenge read with their named priorities and percent volumes.
