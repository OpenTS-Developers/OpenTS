---
format_id: opents-ini
title: OPENTS.INI
summary: Names the folders a deployment keeps its game files sorted into.
kind: file
source_files:
  - code/gamedirs.cpp
filenames:
  - OPENTS.INI
related:
  - type: command
    id: launch:data-directory
  - type: command
    id: launch:user-directory
  - type: using
    id: game-data
---

A distribution ships this file beside its game data to say where that data is kept. It is the deployment's own file, as against `SUN.INI`, which the game writes a player's settings back to.

```ini title="OPENTS.INI"
[Paths]
SearchPaths=INI,MIX,Maps,Addons
```

`SearchPaths` names folders separated by commas, which the game searches in the order written. The whitespace around a name is dropped, a trailing separator is supplied if the name lacks one, and a folder named twice is searched once. Commas separate the entries because a semicolon opens a comment on the line it appears in.

Without the file, and without the key, the game behaves as though `SearchPaths=INI,MIX,Maps` were written: a distribution can sort its files into `INI`, `MIX` and `Maps` folders and ship no configuration at all. A written list **replaces** that default rather than adding to it, so a deployment that wants the default folders as well as its own names them again.

The game's own directory is examined before any listed folder, so naming it adds nothing. Naming only it, as `SearchPaths=.`, is how a deployment asks for no other folder to be searched — an entry with nothing after the equals sign is passed over by the file reader and would leave the default in force.

## Where the file is looked for

The file is read from the disk rather than through the game's file layer, so a deployment cannot describe its own layout from inside an archive. It is looked for in the game data directory, then in that directory's `INI` and `MIX` folders, and the first copy found is the one read.

The game data directory is what [`-DATADIR`](/using/command-line/data-directory/) names, and the game's own directory when nothing names one. Every folder `SearchPaths` lists is relative to it.

## The order files are searched for in

1. the user data directory, when [`-USERDIR`](/using/command-line/user-directory/) names one;
2. the game's own directory;
3. the game data directory, when [`-DATADIR`](/using/command-line/data-directory/) names one;
4. the folders `SearchPaths` lists, in the order written.

Everything the game opens follows that order: archives, rules, artwork, scenarios and launch files alike. A loose file still stands in for an archived one, so a copy found in any of these folders is used ahead of an archived copy of the same name.

A player's own copy is therefore the one the game reads, whatever a deployment ships under the same name. That is what makes a shared installation work: the settings, hotkeys and saved games a player has are theirs, and the rest is read from the copy everyone shares.

Wildcard searches — for rules, battle files, map packs, saved games, map archives and movie archives — cover every directory in the list rather than stopping at the first that holds a match. A name held by more than one is used once, from the one that comes first, which is the same copy an ordinary open of that name would land on.

:::caution[Files the game writes are not searched for]
Settings, saved games, recordings and everything else the game writes go to the user data directory, or to the game's own directory when there is none. A file the game deletes is its own copy, so throwing away a player's hotkeys falls back to the ones a deployment shipped rather than removing them. Nothing listed here is ever written to or deleted from.
:::
