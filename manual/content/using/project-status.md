---
title: Project status
summary: siliconsun publishes playable releases of the OpenTS engine; campaigns, skirmish, saving, and LAN play are functional.
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

OpenTS continues the reconstructed Tiberian Sun engine as an actively developed project. siliconsun publishes a 64-bit Windows client and an Apple Silicon macOS client of that engine, with the engine source and this manual.

Release 0.1.0 runs the full Tiberian Sun 2.03 Firestorm game, with the fixes and changes listed in its release notes. The GDI, Nod and Firestorm campaigns, skirmish, and saving and loading have had full play-through testing. LAN multiplayer is playable and has had more limited testing. No user-visible regression from the original game is currently known. CnCNet play is not yet supported.

## Releases and developer builds

siliconsun releases are published on this repository's GitHub releases page. A release contains the Windows x64 client and the Apple Silicon client.

OpenTS does not distribute the original game assets. They come from an existing Tiberian Sun installation; [Game data](/using/game-data/) covers where a developer build finds them.

## Toolchain and targets

- CMake with Visual Studio 2022
- 32-bit and 64-bit Windows
- C++20
- Debug and Release configurations

Both platforms build in both configurations with the documented toolchain. [Build and run](/using/build-and-run/) gives the commands.
