---
title: Remove CD-ROM-dependent startup behavior
category: feature
release: 0.1.0
breaking: true
migration:
- Check any shortcut carrying `-CD<path>`. It now adds a local data search path rather than selecting a disc.
- Remove `CD=` from map packet entries and from a loose map's `[Multiplay]` section. A map no longer names the discs it may be played from.
targets:
- type: command
  id: launch:cd-path
  effect: changed
- type: key
  id: PlayIntro
  effect: added
- type: key
  id: CD
  effect: removed
  scope: map-packets
- type: key
  id: CD
  effect: removed
  scope: multiplayer-maps
- type: key
  id: CD
  effect: changed
  scope: campaign
credit: [ZivDero]
---

`-CD<path>` now adds a local data search path in both Debug and Release builds.
It no longer selects a CD-ROM or changes which map and movie archives are
mounted.

`[Intro] PlayIntro` replaces the former per-disc intro flags with one startup
setting for `EVA.VQA`. When that path is selected, the game writes the setting
back as `no`.

A campaign still carries its `CD` number, but nothing asks for the disc: it now
decides only whether the opening cinematic plays and which loading backdrop is
shown.
