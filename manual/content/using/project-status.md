---
title: Project status
summary: OpenTS provides playable releases and nightly developer builds; campaigns, skirmish, saving, and LAN play are functional.
category: getting-started
source_files:
  - README.md
  - docs/BUILDING.md
related:
  - type: using
    id: build-and-run
  - type: using
    id: game-data
---

OpenTS is an active continuation of the reconstructed Tiberian Sun engine,
released as a complete standalone `Game.exe` alongside the engine source and
this manual.

Release 0.1.0 delivers the complete Tiberian Sun 2.03 Firestorm game with
the fixes and changes listed in its release notes. The GDI and Nod
campaigns, the Firestorm campaigns, skirmish, and saving and loading are
functional and have received full play-through testing. LAN multiplayer is
functional with more limited testing. No user-visible regression from the
original game is currently known. CnCNet play is not yet supported.

Stable releases are published on the project's GitHub releases page. Nightly
developer builds carry the latest merged changes without release validation,
and their downloads expire after 90 days. OpenTS does not distribute the
original game assets; an existing Tiberian Sun installation provides them.

## Toolchain and targets

- CMake with Visual Studio 2022
- 32-bit and 64-bit Windows
- C++20
- Debug and Release configurations

Every platform and configuration compiles with the documented toolchain.
