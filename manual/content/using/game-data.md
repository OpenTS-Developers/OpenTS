---
title: Game data
summary: A local developer build reads legitimately owned Tiberian Sun data from the repository's Run directory.
category: getting-started
source_files:
  - README.md
  - code/addon.cpp
  - code/gamedirs.cpp
  - docs/BUILDING.md
  - Run/place_steam_build_here
related:
  - type: using
    id: build-and-run
  - type: using
    id: configuration-files
  - type: format
    id: opents-ini
  - type: command
    id: launch:data-directory
  - type: command
    id: launch:user-directory
---

The repository holds the engine source and build inputs. It holds no maps, movies, audio or other proprietary game assets.

Copy the data from a legitimately owned copy of Tiberian Sun into `Run/`. Git ignores everything in `Run/` except the tracked `Run/place_steam_build_here` marker.

Keep the data in `Run/`, not in the CMake build directory. A build writes its output only to `build/bin/<configuration>/` and copies nothing into `Run/`. [Build and run](/using/build-and-run/) shows how to launch a build with `Run/` as its data directory.

The game offers Firestorm only when it finds `FIRESTRM.INI`, either as a loose file or inside `PATCH.MIX`, `PCACHE.MIX`, or an `EXPAND` or `ECACHE` [archive](/formats/mix/). No other expansion file is checked. Without `FIRESTRM.INI` only the base game can be played, and with it Firestorm is offered even when other expansion files are missing. `RulesExpansion` in [`OPENTS.INI`](/formats/opents-ini/) can name a different file.

## Keeping the data somewhere else

[`-DATADIR=<path>`](/using/command-line/#data-directory) names the game data directory, which the game reads its data from and never writes to. Without it, the game reads its data from the directory that holds the executable.

[`-USERDIR=<path>`](/using/command-line/#user-directory) names the user data directory, which receives every file the game writes, including settings, saved games, recordings and downloaded maps. Without it, the game writes these files beside the executable. The exceptions are the debug log, [out-of-sync reports](/using/out-of-sync-reports/) and [crash reports](/using/crash-reports/), which always go into folders beside the executable.

A macOS game launched from an application bundle, with neither option set, reads its data from the home directory's `Games/OpenTS` folder and writes the player's files to the home directory's `Library/Application Support/siliconsun` folder. An option that is passed still names its own directory. The data directory still has to exist already. Any other launch keeps the directories above.

When the game looks for a file, it checks the user data directory first, so a player's copy is read ahead of a shipped copy with the same name. One copy of the data can therefore serve several players, each with a user data directory that holds their own settings and saves. [`OPENTS.INI`](/formats/opents-ini/#the-order-files-are-searched-for-in) gives the full search order and the files it does not apply to.

[Saved games](/formats/save-games/) are kept in a `Saved Games` folder in the user data directory, and [screen captures](/commands/screencapture/) in a `Screenshots` folder beside it. The game lists and loads saves only from that folder, and never looks in either folder for any other file.

The data can be sorted into folders inside the data directory. Without configuration, the game also searches the `INI`, `MIX` and `Maps` folders there. `SearchPaths` in [`OPENTS.INI`](/formats/opents-ini/) names a different list and sets the order the folders are searched in.
