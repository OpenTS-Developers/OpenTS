# OpenTS on macOS

A personal fork of [OpenTS](https://github.com/OpenTS-Developers/OpenTS), the
open-source reconstruction of *Command & Conquer: Tiberian Sun*, with an
experimental Apple Silicon build so I can play the game natively on macOS.

<img src="menu.png" alt="Tiberian Sun main menu running on Apple Silicon" width="900">

<img src="gameplay.png" alt="Tiberian Sun in-game on Apple Silicon" width="900">

## What this is

OpenTS rebuilds the Tiberian Sun engine as a standalone program instead of
patching the retail executable. This fork keeps the upstream engine and adds a
working arm64 macOS build on top of it. I use it to play Tiberian Sun on a Mac.

Everything about the project itself — what OpenTS is and is not, its status and
roadmap, the manual, history, and license — lives in the
[upstream README](https://github.com/OpenTS-Developers/OpenTS) and its
[documentation](https://github.com/OpenTS-Developers/OpenTS/tree/main/docs).
This file covers only what differs here: building and running on macOS.

## Building on macOS (Apple Silicon)

The macOS build is experimental and unsupported. It builds with Apple Clang and
Ninja on arm64. Windows with Visual Studio 2022 is still the supported target;
see the upstream
[BUILDING.md](https://github.com/OpenTS-Developers/OpenTS/blob/main/docs/BUILDING.md).

1. Install CMake and Ninja:

   ```bash
   brew install cmake ninja
   ```

2. Fetch the submodules:

   ```bash
   git submodule update --init --recursive
   ```

3. Configure and build a Debug binary:

   ```bash
   cmake -S . -B build/macos -G Ninja \
     -DCMAKE_BUILD_TYPE=Debug \
     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   cmake --build build/macos
   ```

   The binary is `build/macos/bin/GameD`.

4. Point the game at a Tiberian Sun data directory and run:

   ```bash
   build/macos/bin/GameD -DATADIR=/path/to/tibsun
   ```

The data directory needs the original Tiberian Sun assets, for example from
Command & Conquer The Ultimate Collection on Steam or the EA App. OpenTS ships
only the engine, never the game data.

## License

This fork keeps the upstream license: GPL-3.0-or-later, with the additional
Section 7 terms for material derived from Electronic Arts source. See
[LICENSE.md](LICENSE.md).
