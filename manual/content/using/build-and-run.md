---
title: Build and run
summary: Builds the Debug or Release executable for either platform and runs it against a directory of game data.
category: getting-started
source_files:
  - docs/BUILDING.md
  - CMakeLists.txt
  - code/CMakeLists.txt
  - code/language/CMakeLists.txt
related:
  - type: using
    id: game-data
  - type: using
    id: developer-build-troubleshooting
---

Install Visual Studio 2022 with the **Desktop development with C++** workload, a Windows SDK, CMake 3.23 or newer, and Git for Windows. The repository's `docs/BUILDING.md` covers toolchain details and options.

The renderer is a vendored dependency, so a clone that did not fetch submodules has to fetch them before configuring. Configuration stops with instructions if they are missing.

```powershell title="PowerShell"
git submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug
```

The Debug build writes `GameD.exe`, its symbols, map file, and the matching `Language.dll` to `build/bin/Debug/`, and places the repository's `ui/` directory of UI documents, styles, and font beside them. Use `--config Release` to write `Game.exe` to `build/bin/Release/` instead. Nothing is copied out of the build directory, so the two configurations never overwrite each other.

`-A x64` builds the 64-bit executable. A build directory holds one platform, so give the 64-bit build its own, such as `-B build/x64`. A saved game belongs to the platform that wrote it, and a network game needs every player on the same platform.

Supply the required game data in `Run/`, then launch the built executable and name that data directory:

```powershell title="PowerShell"
.\build\bin\Debug\GameD.exe -DATADIR=Run
```

The engine reads every string and dialog it displays from `Language.dll`, and loads it from the directory holding the executable. The freshly built copy is therefore the one that runs, and a localized or edited library sitting in the game data directory is not read.
