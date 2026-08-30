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

A file that seats more than one person asks for a game against other machines.

## Resuming a saved game

`LoadSaveGame=yes` resumes the saved game `SaveGameName` names, and settles the question by
itself: a saved game carries the kind of game it was, the options it was played under and
the houses that played it, so nothing else in the file decides those. A client resuming a
campaign writes little more than the name of the save.

The name is a file inside the game's saved-games folder, and a name written with a path of
its own is reduced to its last part. A save the folder does not hold, or one made by
another version of the game, refuses the launch with the reason shown.

A save from a game against other machines resumes as well. Every machine loads its own
copy of the save — the synchronized in-game save writes one on each of them, named
`SAVEGAME.NET` — while the file seats the same people again, with the addresses their
machines answer on now. A player who does not return leaves their house fighting on under
the computer, and before play resumes the machines compare the games they loaded, so
mismatched saves refuse rather than drift apart. The launch is refused when the seats and
the save disagree on who is playing, or when the save came from a game the menu arranged
over the local network.

## A campaign mission

`IsSinglePlayer=yes` plays the mission `Scenario` names. `CampaignID` says which campaign
the mission belongs to, counted from zero in the order the battle files declare them, or
`-1` for a mission outside any campaign. The campaign decides what the mission leads on to
and which ending it plays, and it is what the game's own introduction is gated on, exactly
as when a campaign is chosen from the menu.

`DifficultyModeHuman` and `DifficultyModeComputer` each name a difficulty from 0 to 2 — the
player's houses and the computer's, applied independently, so all nine pairings can be
played where the menu offers only its three. A restart or the next mission keeps the pair.

`[GlobalFlags]` seeds the scenario flags a mission chain carries forward: entries
`GlobalFlag0` through `GlobalFlag49` are set on the mission as it starts, so a mission
launched partway through a chain begins in the state the missions before it left.

The mission's own briefing and opening movies play as they do from the menu; only the
game's startup movies are skipped.

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

## A game against other machines

Each machine writes its own file, with itself in `[Settings]` and everybody else in the
`[OtherN]` sections. Those sections carry `Ip` and `Port` as well, naming the address a
machine answers on. A `[Tunnel]` section with its own `Ip` and `Port` routes the match
through a tunnel instead, and each machine is then named by the tunnel number its own `Port`
key carries rather than by its address.

Every person must be named, and no two may be named the same, whatever the letters' case.
The seats are ordered by color, and a name is what breaks a tie between two of one color, so
a match without those names is not the same match on every machine. Colors themselves may
still be shared.

The seed is taken exactly as written, the same on every machine — including `0`, which in a
match against other machines is a seed like any other rather than a draw from chance.

When a `[Tunnel]` section names a server, the match is played through it; otherwise each
machine is reached straight at the address its section carries, while this machine listens
on the port its own `Port` key names.

## When something is wrong

A file describing a game that cannot be played is refused: the reason is shown and written
to the log, and the game exits rather than falling back to its menu. A launch is refused
when it asks for more computer players than there are seats, names a country or color the
loaded rules do not have, names a difficulty that is not one, allies a seat with one the
match does not hold, or asks for a seat that watches rather than plays. A match against
other machines is refused as well when a person is left unnamed or two are named the same.

A difficulty easier than the three the game has is not refused: the seat is played at the
easiest one the game does have.

## What the game does not take from a launch file

The timing keys are not read at all, `ReconnectTimeout` and `ConnTimeout` among them. How
far ahead the machines run, how often they exchange their orders, and how long they wait for
one that has gone quiet are the game's own business, and no launch file changes them.
`MapHash` is not read either: the machines compare the games they have loaded before play
begins, which settles the same question for themselves.

These keys are read but do not change anything yet, each awaiting the behavior that will
honor it: `IsHost`, `Tournament`, `GameID`, `WriteStatistics`, the automatic-save scheduling
keys, `BuildOffAlly`, `AttackNeutralUnits`, `ScrapMetal`, `AutoSurrender`,
`ContinueWithoutHumans`, `CoachMode`, `QuickMatch`, `SkipScoreScreen`,
`PlayMoviesInMultiplayer`, `CustomLoadScreen`, `CustomLoadScreenPos`, and
`DifficultyName`.
