---
format_id: spawn-ini
title: Client launch file
summary: Describes the match a client asks the game to launch when it starts the game with -SPAWN.
kind: file
source_files:
- code/spawnerconfig.cpp
- code/spawnerconfig.h
- code/spawner.cpp
filenames:
- SPAWN.INI
related:
- type: format
  id: ini-syntax
- type: command
  id: launch:spawn
---

A client that sets up matches outside the game writes this file beside the game and starts
the game with [`-SPAWN`](/using/command-line/spawn). The game then plays the match the file
describes instead of showing its own menu, and exits when that match ends.

The vocabulary below is the client's, not the game's: the spelling of every key, and what
each means when it is left out, are settled by what clients already write. Reading the file
never fails. A key the game does not know is passed over, a value it cannot make sense of
keeps the meaning an absent key would have, and whether the result describes a game that
can be played is judged once, at the moment the launch is attempted.

## What the file asks for

The `[Settings]` section says what kind of game to start.

| Key | Meaning |
| --- | --- |
| `Scenario` | The scenario file to play. Defaults to `spawnmap.ini`. |
| `IsSinglePlayer` | Play a campaign mission rather than a match. |
| `LoadSaveGame`, `SaveGameName` | Resume the named saved game. |

Only a skirmish is currently launched this way. A file asking for a campaign mission, a
saved game, or a game against other machines is refused with the reason shown, and the game
exits.

## The options every house plays under

Read from `[Settings]`: `Bases`, `Credits`, `BridgeDestroy`, `Crates`, `ShortGame`,
`GameSpeed`, `MultiEngineer`, `UnitCount`, `AIPlayers`, `AIDifficulty`, `AlliesAllowed`,
`FogOfWar`, `MCVRedeploy`, `TechLevel`, `Firestorm`, and `Seed`.

A written `Seed` makes a launch repeatable: the same file played twice places every house
the same way. A seed of `0` leaves the placement to chance, which is also what an absent
`Seed` means.

`HarvesterTruce` is read and recorded with the rest of the match's options, but a skirmish
takes harvester immunity from the scenario's own `[SPECIAL]` section, so the key does not
change how a skirmish is played.

## Who is playing

A seat is a person's because the file writes a section for it: `[Settings]` describes the
player at this machine, and `[Other1]` through `[Other7]` describe the others. Each names
`Name`, `Side` (the country), and `Color`.

A seat no section claims is a computer player, described by position instead:

| Section | Entry | Meaning |
| --- | --- | --- |
| `[HouseColors]` | `Multi1`–`Multi8` | The color that seat plays. |
| `[HouseCountries]` | `Multi1`–`Multi8` | The country that seat plays. |
| `[HouseHandicaps]` | `Multi1`–`Multi8` | The difficulty that seat plays at. |

A computer seat may write `-1` for its country or color and leave the choice to the game,
as a game set up from the menu does. A person's seat names both. `AIPlayers` says how many
of the unclaimed seats are actually played by a computer.

The seats are then ordered the way the game creates houses — the people first, by ascending
color — and everything below that names a seat by number means that order.

| Section | Entry | Meaning |
| --- | --- | --- |
| `[SpawnLocations]` | `Multi1`–`Multi8` | The map start position that seat begins at. |
| `[Multi1_Alliances]`–`[Multi8_Alliances]` | `HouseAllyOne`–`HouseAllyEight` | The seats that seat is allied with. |

A start position the map does not declare, or one another seat has already taken, is left
to the game to choose, which is also what writing no position means. Alliances are made
exactly as written, before the first frame is played, and quietly: a match whose file
forbids new pacts still starts with the ones it wrote.

Two seats may share a color deliberately — a cooperative team does — and the game does not
refuse it.

## When something is wrong

A file describing a game that cannot be played is refused: the reason is shown and written
to the log, and the game exits rather than falling back to its menu. A launch is refused
when it asks for more computer players than there are seats, names a country or color the
loaded rules do not have, names a difficulty that is not one, allies a seat with one the
match does not hold, or asks for a seat that watches rather than plays.

A difficulty easier than the three the game has is not refused: the seat is played at the
easiest one the game does have.

## What the game does not take from a launch file

The file's timing keys are not read at all. How far ahead the machines run and how often
they exchange their orders is the game's own business, and no launch file changes it.

These keys are read but do not yet change anything, because the game has no such behavior
to give them to: `Tournament`, `GameID`, `MapHash`, `BuildOffAlly`, the automatic-save
keys, `QuickMatch`, `SkipScoreScreen`, `WriteStatistics`, `CoachMode`, `AutoSurrender`,
`AttackNeutralUnits`, `ScrapMetal`, `ContinueWithoutHumans`, `PlayMoviesInMultiplayer`,
`CustomLoadScreen`, `CustomLoadScreenPos`, and `DifficultyName`. The tunnel and timeout
keys describe how machines reach one another, which a skirmish never needs.
