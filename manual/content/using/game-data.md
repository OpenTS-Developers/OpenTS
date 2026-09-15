---
title: Game data
summary: A local developer build reads legitimately owned Tiberian Sun data from the repository's Run directory.
category: getting-started
source_files:
  - README.md
  - code/addon.cpp
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

The repository contains engine source and build inputs. It does not contain maps, movies, audio, or other proprietary game assets.

Place data from a legitimate copy of Tiberian Sun under `Run/`. The tracked `Run/place_steam_build_here` marker identifies this local run tree; the directory's populated contents are ignored by Git.

Firestorm counts as installed when the game finds `FIRESTRM.INI`. That one file decides it, so a deployment keeping the expansion's content in its own archives is still offered Firestorm, and one without that file can only play the base game.

Do not place game data in the CMake build directory. The build writes the executable and `Language.dll` into `build/bin/<configuration>/` and copies nothing into `Run/`, so the build tree holds only what the build produced.

## Keeping the data somewhere else

`-DATADIR=<path>` reads the game's data from the directory named instead of requiring it beside the executable. `-USERDIR=<path>` keeps what the game writes, including settings, saved games, recordings and downloaded maps, in a directory of its own. Together they let one copy of the data serve several people, each writing only to their own directory and reading their own files ahead of the shared ones.

[Saved games](/formats/save-games/) go one step further, into a `Saved Games` folder of their own inside that directory. That folder is not searched, so a save is written, listed and deleted by name in it rather than looked for across the folders the game reads from.

The data may be sorted into folders rather than left in one directory. Without any configuration the game also searches `INI`, `MIX` and `Maps`; [`OPENTS.INI`](/formats/opents-ini/) names other folders and the order they are searched in.
