---
format_id: save-games
title: Save games
summary: Stores versioned OpenTS game state in `.SAV` and `.NET` files.
kind: binary
extensions:
  - .SAV
  - .NET
role: persistence
source_files:
  - code/autosave.cpp
  - code/conquer.cpp
  - code/deploymentconfig.cpp
  - code/desyncdlg.cpp
  - code/event.cpp
  - code/goptions.cpp
  - code/init.cpp
  - code/loaddlg.cpp
  - code/mainloop.cpp
  - code/mpload.cpp
  - code/netdlg.cpp
  - code/savefile.cpp
  - code/saveload.cpp
  - code/savemgr.cpp
  - code/savestream.cpp
  - code/savever.cpp
  - code/scenfile.cpp
  - code/abstract.cpp
  - code/objtype.cpp
  - code/unittype.cpp
  - code/ambient.cpp
  - code/voc.cpp
---

A saved game is a file in a format specific to OpenTS: `.SAV` for campaigns and skirmishes, and `.NET` for [games against other machines](#numbered-multiplayer-saves). Each file begins with a fixed header and a table of the details the load dialog lists a save by. The game state follows as one block, compressed when compression makes it smaller. The load dialog reads only the header and the table to list a save.

The game writes each save under a temporary name and moves it into place once every byte is on disk, so an interrupted save leaves the previous file intact.

## Where the files are

Every saved game is kept in a `Saved Games` folder, beside the game or inside the [user directory](/using/game-data/) when one is named. The game creates the folder when it first needs it. Saving, loading, listing and deleting all use this one folder. Unlike game data files, a saved game is never searched for in other folders. A client that browses saved games should read this folder, whichever layout the game was installed in.

The save dialog names a new save `SAVE` followed by four random hexadecimal digits, and picks again if a file already has that name. Saving over a listed game reuses that game's file name. A multiplayer save uses a [numbered name](#numbered-multiplayer-saves) instead and never appears in the campaign or skirmish list.

The random map generator keeps the settings a player saves in the same folder, under other names. The settings file for a random map played in a match is not kept there. It is written with the game's other files so that the host can send it to the other players.

## When the file is written

In a campaign or skirmish, the save dialog writes the file at once, while the dialog has the scenario paused. If the save fails, the dialog shows an error box and stays open.

In a game against other machines, the Save button in the options menu sends a `SAVEGAME` command to every machine. When the command executes, each machine writes the save at the end of that frame, after the objects destroyed during the frame have been removed. Several save commands in one frame produce one save.

The [Create Autosave](/mapping/actions/taction-create-autosave/) trigger action requests an automatic save. The save is written at the end of the frame in which the action runs, under the names and descriptions given in [Automatic saves](#automatic-saves). A trigger save posts a message only when it fails.

The action works even when timed saves are turned off, including in a game against other machines started from the menu. Several requests in one frame produce one save. Nothing is saved while a recording plays back.

In a game against other machines, a trigger save or timed save written in the frame of a player's save command joins the player's save. That save keeps the player's file name, description, saving box, and `Game saved.` message.

Multiplayer saving is disabled for the rest of the match once a human player leaves or loses their connection. Any save still waiting to be written is cancelled, and the options menu grays out its Save button. Saving is allowed again when a new game is selected and started, or when the [master](/systems/out-of-sync-recovery/#the-master) [loads a save of the match](#loading-during-a-match).

A save reports its outcome in the message list. A save the player asked for shows `Game saved.` or `The game could not be saved.` The line appears at the end of the frame, so a save made from the menus is reported once the player is back on the map. In a game against other machines, each machine reports the file it wrote. The [random map generator](/systems/map-generation/) is the exception: its save dialog confirms with a box, since it saves settings, not a game, and runs with no scenario loaded.

## Automatic saves

The game also saves at a fixed interval, counted in frames. A game started from the menu uses [`AutoSaveInterval`](/keys/autosaveinterval/), and a [client-launched](/formats/spawn-ini/#automatic-saves) game uses the interval in its launch file. The interval starts over after every completed save, whoever requested it, and whenever a scenario starts, restarts or is loaded. A new, restarted or resumed game therefore waits a full interval before its first automatic save.

When the interval runs out, `Auto-saving...` appears in the message list. The save is written at the end of the next frame, after that line has been drawn. The same line then changes to `Game auto-saved.` or `The game could not be auto-saved.`, so a timed save leaves one line in the list whatever its outcome. In a game against other machines, the `Auto-saving...` line is not replaced when the save [joins a player's save](#when-the-file-is-written), or when saving is disabled or a load is scheduled before the save is written.

A campaign writes `AUTOSAVE1.SAV` through `AUTOSAVE5.SAV` in turn and then starts again at `AUTOSAVE1.SAV`. A skirmish does the same with `AUTOSAVE_SKIRMISH1.SAV` through `AUTOSAVE_SKIRMISH5.SAV`, and the two sequences advance independently. Each file is described as `Auto-Save N: ` followed by the scenario's description, where `N` is the file's number, so the list tells it apart from a save the player named.

Every save records the next number of both sequences, and loading a save continues both from there. Otherwise the game keeps its place in each sequence for as long as it runs. A new game started from the menu therefore continues where the last automatic save left off and does not overwrite it. A client-launched game starts at the numbers its launch file names.

In a game against other machines, timed saves run only when a launch file set the interval. A match started from the menu has no timed saves, because each player's `AutoSaveInterval` can differ and every machine must save at the same frame. In a client-launched match, each machine writes the next [numbered save](#numbered-multiplayer-saves), described as `Multiplayer Game (Auto-Save)`, without the saving box a manual save shows. Timed saves stop once multiplayer saving is disabled for the match.

## Quick saves

The [`QuickSave`](/commands/quicksave/) command saves a campaign to `QUICKSAVE.SAV` and a skirmish to `QUICKSAVE_SKIRMISH.SAV`. Each replaces the previous quick save of its kind, so a skirmish quick save never replaces a campaign one. The file is written at the end of the frame in which the key was pressed, behind the saving box a menu save shows. The message list then shows `Game saved.` or `The game could not be saved.`

A quick save is described as `Quick Save: ` followed by the scenario's description, and the load dialog lists it like any other save. Like any completed save, it starts the automatic-save interval over.

[`QuickLoad`](/commands/quickload/) loads the quick save of the kind of game being played. The pages of the two commands list when each one is ignored. Neither command has a default key.

## Numbered multiplayer saves

A game against other machines writes every save, timed or from the options menu, as `SVGM_nnn.NET`. The number `nnn` is the lowest from `000` that no file in the folder uses, so a match's saves count up in step on every machine. When every number up to `999` is taken, `SVGM_999.NET` is overwritten.

Starting a new match deletes the numbered saves and the `spawnSG.ini` a previous match left. A resumed match keeps its files and continues the numbering.

A client-launched match copies its launch file into the folder as `spawnSG.ini` at its first successful save. The CnCNet client reads that copy to resume the match. If the copy cannot be written, the next save tries again.

The game no longer writes `SAVEGAME.NET`. Clients used to wait for a save under that name, rename it, and do the cleanup and copy themselves.

## Loading during a match

In a game against other machines, the master can load one of the match's saves while the match is running, from the options menu or from the [out-of-sync dialog](/systems/out-of-sync-recovery/). The list shows numbered saves written by this version in the same kind of network game. Only the 32 most recently written `.NET` files in the folder are read for the list.

Picking a save starts a five-second countdown on every machine. Each machine then loads the save with the same number from its own `Saved Games` folder:

1. It discards the commands still in transit for the running match and restores the save.
2. It gives each seated player the saved house with that player's name.
3. It reconnects to the other machines and compares its restored game with theirs, as a resumed save does. If the games differ, the match stops with a mismatch message.

A player who has left since the save was written has their house played by the computer. The loaded match plays at the speed it was playing at when the save was written.

No save is written while a load is pending. A machine that cannot load the save shows the loading error and leaves the match.

## What the file holds

The listing table holds the description shown in the list, the player's house, the campaign and scenario numbers, and the game type. It also holds three timestamps, all set to the time of the save, a game-version field that is always 1, the program name `SUN.EXE`, and the OpenTS version stamp described in [What is checked](#what-is-checked). The header holds the version of the file format itself. The load dialog lists saves newest first by the file's last-modified time, not by these timestamps.

The game state is a fixed sequence of records, written and read in the same order. It opens with the scenario, the environment, the rules, the animation types and the map. The global values and the lists of the other type definitions and runtime objects follow, and outside a campaign the game options come last. Each list stores its length ahead of its members, and each member stores its fields one by one, so the file does not depend on how objects are laid out in memory.

The type definitions are stored in the save. A loaded game therefore uses the rules types it was saved with, not those in the current rules files. Artwork is not stored: once a type is restored, its shapes and voxel models are loaded again from the game's files, so a save loaded against changed files shows the current artwork.

The global values include the looping sounds that [Play Sound Effect At](/mapping/actions/taction-play-sound-at/) left at waypoints, each stored as the sound and its location. The sound that was playing is not stored. It starts again once its location is within hearing range after the load.

When [`CarryScenarioFile`](/formats/opents-ini/#what-a-save-carries) in `OPENTS.INI` asks for it, the scenario record also holds the scenario file itself, name and contents. The record has room for the file either way and is left empty when the file is not included. A [restart or replay](/systems/campaign-progression/#losing-and-restarting) after a load reads this copy, not the file on disk, which a client resuming the save may have replaced. A random map has no file to include.

## What is checked

A save is offered and loaded only when its version stamp matches the running version of OpenTS. The load dialog reads the header and listing table of every `.SAV` file in the `Saved Games` folder and skips any file stamped by another version, including another OpenTS release. Tiberian Sun and earlier OpenTS releases wrote saves in a compound-document layout, which this format does not recognize, so those files are skipped as well. There is no conversion.

A save loaded without the dialog, such as a network save or one resumed from a [launch file](/formats/spawn-ini/), is refused in the same way when its stamp differs. Development snapshots of one version share its stamp until a change to the save layout or the simulation raises a serial the stamp includes. A snapshot whose layout changed without that raise can still list a save it fails to load.

Apart from the version stamp and the add-on the saved scenario requires, a save is not compared with the game loading it. A save made under one set of rules and loaded under another is not detected. The type definitions stored in the file replace the ones the current rules built.

The whole file, checksums included, is read and checked before the game in progress is cleared. A file that is truncated, damaged, or written in a later format version is therefore refused before anything is replaced. Restoring then clears the game in progress and stops at the first record it cannot restore, or when the add-on the saved scenario requires is not installed. After the last record it also fails when a reference between objects refers to an object the file does not contain. A load that stops at any of these points fails, and the game in progress is already gone.
