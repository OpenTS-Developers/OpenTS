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

Firestorm counts as installed when the game finds `FIRESTRM.INI`. That one file decides it, so a deployment keeping the expansion's content in archives of its own is still played as the expansion, and one without that file is played as the base game.

Do not place game data in the CMake build directory. The build copies OpenTS executables and `Language.dll` into `Run/`, alongside the locally supplied game files.

## Keeping the data somewhere else

`-DATADIR=<path>` reads the game's data from the directory named instead of requiring it beside the executable, and `-USERDIR=<path>` keeps what the game writes — settings, saved games, recordings and downloaded maps — in a directory of its own. Together they let one copy of the data serve several people, each writing only to their own directory and reading their own files ahead of the shared ones.

The data may be sorted into folders rather than left in one directory. Without any configuration the game also searches `INI`, `MIX` and `Maps`; [`OPENTS.INI`](/formats/opents-ini/) names other folders and the order they are searched in.
