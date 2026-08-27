---
title: Build and run
summary: Builds the 32-bit Debug or Release executable and copies it into the local Run directory.
category: getting-started
source_files:
  - docs/BUILDING.md
  - CMakeLists.txt
  - code/CMakeLists.txt
related:
  - type: using
    id: game-data
  - type: using
    id: developer-build-troubleshooting
---

Install Visual Studio 2022 with the **Desktop development with C++** workload, its **C++ ATL** and **C++ MFC** components — the workload does not select them on its own — a Windows SDK, CMake 3.23 or newer, and Git for Windows. The repository's `docs/BUILDING.md` covers toolchain details and options.

The renderer is a vendored dependency, so a clone that did not fetch submodules has to fetch them before configuring. Configuration stops with instructions if they are missing.

```powershell title="PowerShell"
git submodule update --init --recursive
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug
```

The Debug build copies `GameD.exe`, its symbols, map file, and the matching `Language.dll` into `Run/`. Use `--config Release` to produce `Game.exe` instead.

After supplying the required game data in `Run/`, launch the selected executable from that directory:

```powershell title="PowerShell"
.\Run\GameD.exe
```

Building one configuration after the other replaces `Run/Language.dll` with the matching configuration's copy. Every string and dialog the engine displays is read from that library, so a `Language.dll` supplied by a localized or edited installation is overwritten by the build and none of its text reaches the screen.
