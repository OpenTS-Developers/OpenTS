---
title: Developer-build troubleshooting
summary: Checks the supported toolchain, target architecture, output location, and local game-data tree.
category: troubleshooting
source_files:
  - docs/BUILDING.md
  - CMakeLists.txt
  - code/CMakeLists.txt
related:
  - type: using
    id: build-and-run
  - type: using
    id: game-data
---

## Configuration fails before compilation

A message that `thirdparty/bgfx.cmake` is empty means the clone did not fetch the vendored renderer. Run `git submodule update --init --recursive` and configure again.

Use the Visual Studio 2022 generator and `-A Win32`. The build supports no other compilers, Visual Studio versions, or target architectures.

For a Visual Studio installation that CMake cannot discover through the Visual Studio Installer, pass its installation path and product version as described in the repository's `docs/BUILDING.md`.

## The executable is not in the run directory

Builds write their runnable files to `build/bin/<configuration>/` and copy nothing into `Run/`:

- Debug: `GameD.exe`, `GameD.pdb`, `GameD.map`, and `Language.dll`
- Release: `Game.exe`, `Game.pdb`, `Game.map`, and `Language.dll`

## The executable cannot initialize game data

Name the game data directory with `-DATADIR=`, and confirm it holds data from a legitimate Tiberian Sun installation. The repository and CMake build directory do not supply proprietary game assets.
