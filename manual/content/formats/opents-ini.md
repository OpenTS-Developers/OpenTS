---
format_id: opents-ini
title: OPENTS.INI
summary: Names the folders a deployment sorts its game files into, the names of the files the game reads, and what its saves hold.
kind: file
source_files:
  - code/deploymentconfig.cpp
  - code/gamedirs.cpp
  - code/init.cpp
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

`OPENTS.INI` belongs to the deployment and describes how it lays out the game's files. A player's options are kept in a separate settings file, `SUN.INI` unless `Settings` below names another. The game reads `OPENTS.INI` once at startup.

```ini title="OPENTS.INI"
[Paths]
SearchPaths=INI,MIX,Maps,Addons
```

`SearchPaths` lists extra folders the game searches for its files, in the order written. Separate the names with commas. Do not use semicolons: a semicolon starts a comment, so the rest of the line is ignored. Spaces around a name are ignored, a name may end in a backslash or not, and a folder written twice is searched once. Each folder is relative to the game data directory described [below](#where-the-file-is-looked-for).

Without the file, or without the key, the game searches `INI`, `MIX` and `Maps`, as though `SearchPaths=INI,MIX,Maps` were written. A distribution can therefore sort its files into those three folders and ship no configuration at all. A written list replaces the default, so a deployment that wants those folders as well as its own must name them again.

`SearchPaths=.` adds no folder to the search. The `.` entry names the game data directory, which is always searched already, so the game skips it. An empty `SearchPaths=` does not have this effect: the game ignores a key with nothing after the equals sign, so the default list stays in force.

## The files it reads

```ini title="OPENTS.INI"
[Files]
Rules=RULES.INI
RulesExpansion=FIRESTRM.INI
Art=ART.INI
ArtExpansion=ARTFS.INI
AI=AI.INI
AIExpansion=AIFS.INI
Sound=SOUND.INI
SoundExpansion=SOUND01.INI
Theme=THEME.INI
ThemeExpansion=THEME01.INI
Battle=BATTLE.INI
BattleExpansion=BATTLEFS.INI
LanguageRules=LANGRULE.INI
LanguageRulesExpansion=LANGFS.INI
MultiplayerRules=MPLAYER.INI
MultiplayerRulesExpansion=MPLAYERFS.INI
Tutorial=TUTORIAL.INI
UI=UI.INI
Settings=SUN.INI
```

Each key names one file, and a key left out keeps the name shown above.

| Key | File it names |
| --- | --- |
| `Rules` | The rules |
| `Art` | The artwork definitions |
| `AI` | The computer player's data |
| `Sound` | The [sound registry](/formats/sound-ini/) |
| `Theme` | The [music registry](/formats/theme-ini/) |
| `Battle` | The campaign list |
| `LanguageRules` | The translated rules, read over the other rules |
| `MultiplayerRules` | The [rules read only outside a campaign](/formats/multiplayer-rules/) |
| `Tutorial` | The [numbered text lines](/formats/tutorial-ini/) |
| `UI` | The [interface settings](/formats/ui-ini/) |
| `Settings` | The file a player's options are read from and saved to |

Each of the eight `Expansion` keys names the expansion's copy of the matching base file, which is read after the base file. The expansion's rules, AI and multiplayer rules files are used only while Firestorm is enabled, and so is its art file, except for that file's `[Movies]` list. Its sound, music, campaign and translated rules files are read whether or not Firestorm is enabled.

`RulesExpansion` also decides whether the expansion is installed: the game counts Firestorm as installed only when it finds the file this key names.

These keys change only a file's name. The game looks for each named file in the [search order](#the-order-files-are-searched-for-in) below, as it does for every other file.

The game reads its rules from the one file `Rules` names. Other files matching `RULE*.INI` are not read.

Campaign files are gathered by the fixed pattern `BATTLE*.INI`. Every matching file is read and adds its campaigns to the list. The file `Battle` names is read as well when its name does not match the pattern.

## The palettes it starts with

```ini title="OPENTS.INI"
[Palettes]
Scheme=UNITSNO.PAL
Game=TEMPERAT.PAL
```

`Scheme` names the palette player colors are built against, and `Game` names the palette the game is drawn with. They apply from startup until a scenario loads its theater. The theater then replaces `Game` with the palette its [`Root=`](/keys/root/) names. It replaces `Scheme` with its `UNIT` palette, such as `UNITSNO.PAL` for a [`Suffix=`](/keys/suffix/#scope-theater) of `SNO`, when it sets a `Suffix=` and that palette is found. These two names belong in this file because the game needs them before any theater is known.

Two kinds of object are drawn with the `Scheme` palette read at startup, whatever the theater: terrain objects with [`SpawnsTiberium=yes`](/keys/spawnstiberium/), and voxel animations that have no owner.

The game reads these palettes only from archives it caches at startup, such as `CACHE.MIX`, so a loose palette file is not used. [MIX archives](/formats/mix/#caching) covers which files this applies to. If a named palette is not found, the game does not load it, writes the missing name to the debug log, and starts anyway.

## What a save carries

```ini title="OPENTS.INI"
[Saves]
CarryScenarioFile=yes
```

With `CarryScenarioFile=yes`, the game keeps a copy of the scenario file when a mission loads and stores it in every [saved game](/formats/save-games/#what-the-file-holds) of that mission. A generated random map has no file, so its saves hold no copy. Restarting the mission reads that copy, including after the save is loaded. With the default, `no`, a save holds no copy and a restart reads the scenario file from disk again.

Use the copy where the file on disk may have changed since the mission started. A client resuming a save replaces `spawnmap.ini` with a stub, and a restart that reads the stub fails.

The copy makes every save larger, by about the compressed size of the scenario file.

Each save keeps what it was written with. Turning the key off makes new saves smaller and does not change existing ones, and a save that holds a copy still restarts from it.

## Where the file is looked for

`OPENTS.INI` must be a loose file, because the game does not read it from an archive. The game looks for it in the game data directory, then in that directory's `INI` and `MIX` folders, and reads the first copy it finds. The user data directory is not searched for it.

The game data directory is the one [`-DATADIR`](/using/command-line/data-directory/) names, or the game's own directory when that option is not used.

## The order files are searched for in

1. the user data directory, when [`-USERDIR`](/using/command-line/user-directory/) names one;
2. the game's own directory;
3. the game data directory, when [`-DATADIR`](/using/command-line/data-directory/) names one;
4. the folders `SearchPaths` lists, in the order written;
5. the `ui` folder in the game's own directory.

The game opens every file in this order, including archives, INI files, scenarios and launch files, and uses the first copy it finds. A loose copy in any of these directories is used ahead of an archived copy of the same name. The exception is data the game reads straight from a cached archive, such as the palettes above and many shapes, where a loose copy is not used; [MIX archives](/formats/mix/#caching) covers it.

Because the user data directory comes first, the game reads a player's own copy of a file ahead of anything a deployment ships under the same name. In a shared installation, each player keeps their own settings and hotkeys, and every other file is read from the shared copy.

A search by pattern covers every directory in the list, and does not stop at the first directory with a match. The game searches this way for rules files, campaign files, map packs, loose multiplayer maps, map archives and movie archives. A name found in more than one directory is used once, from the directory that comes first, which is the same copy an ordinary open of that name reads.

[Saved games](/formats/save-games/) and [screen captures](/commands/screencapture/) are not searched for. Saves are written, listed, loaded and deleted only in a `Saved Games` folder in the user data directory, or in the game's own directory when there is none, so a launcher finds every save in one place. Screen captures go only to a `Screenshots` folder in the same directory.

:::caution[Do not ship a file the game writes in the game's own directory]
Settings, saved games, recordings, screen captures and the other files the game writes go to the user data directory, or to the game's own directory when there is none. The debug log, out-of-sync reports and crash reports always go into folders beside the executable, as [Game data](/using/game-data/#keeping-the-data-somewhere-else) describes. `mpstats.txt` always goes to the game's own directory.

A file the game writes is found ahead of a shipped copy in a later directory, so a player's saved `SUN.INI` is read instead of one shipped in the `INI` folder. When the player's copy of a file such as `KEYBOARD.INI` is removed, the game reads the shipped copy again. Without a user data directory, the game writes and deletes in its own directory, so a copy shipped there is overwritten or removed and cannot be read again.
:::
