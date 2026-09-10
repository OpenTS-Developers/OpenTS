---
format_id: opents-ini
title: OPENTS.INI
summary: Names the folders a deployment keeps its game files sorted into, the files it reads them from, and what its saves carry.
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

This is the deployment's own file, as against `SUN.INI`, which the game writes a player's settings back to.

```ini title="OPENTS.INI"
[Paths]
SearchPaths=INI,MIX,Maps,Addons
```

`SearchPaths` names folders separated by commas, which the game searches in the order written. The whitespace around a name is dropped, a trailing separator is supplied if the name lacks one, and a folder named twice is searched once. Commas separate the entries because a semicolon opens a comment on the line it appears in.

Without the file, and without the key, the game behaves as though `SearchPaths=INI,MIX,Maps` were written: a distribution can sort its files into `INI`, `MIX` and `Maps` folders and ship no configuration at all. A written list **replaces** that default rather than adding to it, so a deployment that wants the default folders as well as its own names them again.

The game's own directory is examined before any listed folder, so naming it adds nothing. Naming only it, as `SearchPaths=.`, is how a deployment asks for no other folder to be searched. An empty `SearchPaths=` does not do this: the file reader passes over an entry with nothing after the equals sign, leaving the default in force.

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
Tutorial=TUTORIAL.INI
UI=UI.INI
Settings=SUN.INI
```

Each key names one file, and an unwritten key keeps the name above. `Rules` names the rules, `Art` the artwork, `AI` the computer player's data, `Sound` the [sound registry](/formats/sound-ini/), `Theme` the [music registry](/formats/theme-ini/), `Battle` the campaign list, `LanguageRules` the translated rules read over the rest, `Tutorial` the [numbered text lines](/formats/tutorial-ini/), `UI` the [interface settings](/formats/ui-ini/), and `Settings` the file a player's own options are written back to.

The seven `Expansion` keys name the expansion's copy of a file, which is read over the base one. `RulesExpansion` also decides whether the expansion is installed: the game looks for that file and nothing else, so renaming it moves the test.

Renaming a file does not move it. Every name here is searched for in the order the section below gives, the same as any other file the game opens.

Rules and campaign files are also gathered by wildcard, as `RULE*.INI` and `BATTLE*.INI`, and those two patterns are fixed. A rules file named outside the pattern is read anyway and is the one the game starts from; where the search turns up others as well, the game asks which set to play with, as it does for a stock installation. Campaign files add to one another, so one named outside the pattern is read alongside those inside it.

## The palettes it starts with

```ini title="OPENTS.INI"
[Palettes]
Scheme=UNITSNO.PAL
Game=TEMPERAT.PAL
```

`Scheme` names the palette the player colors are built against, and `Game` the one the game is drawn through. Both stand only until a scenario loads its [theater](/formats/theater-control/), which replaces them with the palettes the theater's `Root=` and `Suffix=` name. They are settings of this file because nothing has declared a theater yet when they are read.

A palette file the deployment does not ship leaves that palette unchanged, and the name goes to the debug log. The game starts either way.

## What a save carries

```ini title="OPENTS.INI"
[Saves]
CarryScenarioFile=yes
```

`CarryScenarioFile=yes` makes a [saved game](/formats/save-games/#what-the-file-holds) carry the scenario file it was played from, and a restart then reads the mission from that copy rather than from disk. The default is `no`: a save holds no copy and a restart reads the file again.

The copy matters where the file on disk may no longer be the one the mission started from: a client resuming a save replaces `spawnmap.ini` with a stub, and a restart that reads the stub fails. It costs the size of the map, the least compressible part of a save; a large one adds half again to the file.

A save keeps whatever it was written with, so turning the key off shrinks new saves and leaves the old ones as they are.

## Where the file is looked for

The file is read from the disk rather than through the game's file layer, so a deployment cannot describe its own layout from inside an archive. It is looked for in the game data directory, then in that directory's `INI` and `MIX` folders, and the first copy found is the one read.

The game data directory is what [`-DATADIR`](/using/command-line/data-directory/) names, and the game's own directory when nothing names one. Every folder `SearchPaths` lists is relative to it.

## The order files are searched for in

1. the user data directory, when [`-USERDIR`](/using/command-line/user-directory/) names one;
2. the game's own directory;
3. the game data directory, when [`-DATADIR`](/using/command-line/data-directory/) names one;
4. the folders `SearchPaths` lists, in the order written.

Everything the game opens follows that order: archives, rules, artwork, scenarios and launch files alike. A loose file still stands in for an archived one, so a copy found in any of these folders is used ahead of an archived copy of the same name.

A player's own copy is therefore the one the game reads, whatever a deployment ships under the same name. In a shared installation the settings and hotkeys a player has are theirs, and the rest is read from the copy everyone shares.

Wildcard searches — for rules, battle files, map packs, map archives and movie archives — cover every directory in the list rather than stopping at the first that holds a match. A name held by more than one is used once, from the one that comes first, which is the same copy an ordinary open of that name would land on.

[Saved games](/formats/save-games/) are the exception to all of this. They keep to a `Saved Games` folder inside the user data directory, and are named there outright rather than searched for, so that a launcher browsing them finds them in one place.

:::caution[Files the game writes are not searched for]
Settings, saved games, recordings and everything else the game writes go to the user data directory, or to the game's own directory when there is none. A file the game deletes is its own copy, so deleting a player's hotkeys returns the game to the ones a deployment shipped rather than leaving it with none. Nothing listed here is ever written to or deleted from.
:::
