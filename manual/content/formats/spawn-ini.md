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
can be played is judged once, when the launch is attempted.

## What the file asks for

The `[Settings]` section says what kind of game to start.

| Key | Meaning |
| --- | --- |
| `Scenario` | The scenario file to play. Defaults to `spawnmap.ini`. |
| `IsSinglePlayer` | Play a campaign mission rather than a match. |
| `LoadSaveGame`, `SaveGameName` | Resume the named saved game. |

A file that seats more than one person asks for a game against other machines.

## Resuming a saved game

`LoadSaveGame=yes` resumes the saved game `SaveGameName` names, and decides the kind of game
on its own: a saved game carries the kind of game it was, the options it was played under
and the houses that played it, so nothing else in the file decides those. A client resuming a
campaign writes little more than the name of the save.

The name is a file inside the game's saved-games folder, and a name written with a path of
its own is reduced to its last part. A save the folder does not hold, or one made by
another version of the game, is refused, and the reason is shown.

A save from a game against other machines resumes as well. Every machine loads its own
copy of the save — the synchronized in-game save writes one on each of them, named
`SAVEGAME.NET` — while the file seats the same people again, with the addresses their
machines answer on now. A player who does not return leaves their house fighting on under
the computer, and before play resumes the machines compare the games they loaded, so
mismatched saves are refused rather than drifting apart. The launch is refused when the
seats and the save disagree on who is playing, or when the save came from a game the menu
arranged over the local network.

## A campaign mission

`IsSinglePlayer=yes` plays the mission `Scenario` names. `CampaignID` says which campaign
the mission belongs to, counted from zero in the order the battle files declare them, or
`-1` for a mission outside any campaign. The campaign decides what the mission leads on to,
which ending it plays, and which of the game's own introductions plays, exactly as when a
campaign is chosen from the menu.

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
`FogOfWar`, `MCVRedeploy`, `TechLevel`, `Firestorm`, `Seed`, and `CoachMode`.

`CoachMode` decides what a defeated player keeps;
[observers and coach mode](/systems/observers/#coach-mode) owns it.

A written `Seed` makes a launch repeatable: the same file played twice places every house
the same way. A seed of `0` leaves the placement to chance, which is also what an absent
`Seed` means.

`HarvesterTruce` applies in a game against other machines. A skirmish records it with the
rest of the match's options but does not apply it, exactly as a skirmish set up from the
menu does not.

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
| `[IsSpectator]` | `Multi1`–`Multi8` | Whether that seat watches rather than plays. |

A start position the map does not declare, or one another seat has already taken, is left
to the game to choose, which is also what writing no position means. Alliances are made
exactly as written, before the first frame: a match whose file forbids new alliances still
starts with the ones it wrote. Nothing announces them, and the computer players keep the
alliances the file gave them instead of [closing ranks](/keys/paranoid/) as they do against
a side that allies during play. A seat that watches holds no alliances: its own entries and
any naming it are ignored. [Observers and coach mode](/systems/observers/) owns what a
watching seat is shown and how the match treats it.

A computer player may share the color a person plays; in a game against other machines, two
people may not. The client keys each seat by an order no other machine can rebuild, so two
people of one color would take each other's start position and alliances.

## A game against other machines

Each machine writes its own file, with itself in `[Settings]` and everybody else in the
`[OtherN]` sections. Those sections carry `Ip` and `Port` as well, naming the address a
machine answers on. A `[Tunnel]` section with its own `Ip` and `Port` routes the match
through a tunnel instead, and each machine is then named by the tunnel number its own `Port`
key carries rather than by its address.

Every person must be named, and no two may be named the same, whatever the letters' case.
Each machine writes its own file with itself first, so the seats are ordered by color and
name rather than by the order the file wrote them; without those names the machines would
not seat the same match.

The seed is taken exactly as written, the same on every machine — including `0`, which in a
match against other machines is a seed like any other rather than a draw from chance.

When a `[Tunnel]` section names a server, the match is played through it; otherwise each
machine is reached straight at the address its section carries, while this machine listens
on the port its own `Port` key names. Loading progress reaches the other machines with the
in-game retry cadence: a second between retries and ten seconds before a report is given
up.

A tunnel server may run beside the game rather than across the internet, in which case
`[Tunnel] Ip` is the loopback address. The tunnel's port and the number it knows this
machine by are both carried as sixteen-bit values, so each must fall between `1` and
`65535`.

## When something is wrong

A file describing a game that cannot be played is refused: the reason is shown and written
to the log, and the game exits rather than falling back to its menu. A launch is refused
when it

- seats nobody at this machine;
- asks for more computer players than there are seats;
- names a country or color the loaded rules do not have;
- plays the computer at a difficulty the game does not have;
- asks for a game speed the game does not have;
- gives a seat a difficulty outside `-1` to `6`;
- allies a seat with one the match does not hold;
- seats nobody who plays, because every person watches and no computer plays.

A match against other machines is refused as well when

- a person is left unnamed, or two are named the same or given one color;
- a machine other than this one is given no port to answer on or no address to answer at;
- a `[Tunnel]` section carries no address, or one that names no machine;
- `[Tunnel] Port`, or the `[Settings] Port` the tunnel knows this machine by, falls outside
  `1` to `65535`;
- `[Settings] Port` falls outside `1` to `65535` when no tunnel carries the match.

A difficulty easier than the three the game has is not refused: the seat is played as the
easiest opponent the game does have. The two run opposite ways: the easiest opponent plays
at the hardest of the game's three settings.

## What the game does not take from a launch file

The timing keys are not read at all, `ReconnectTimeout` and `ConnTimeout` among them. How
far ahead the machines run, how often they exchange their orders, and how long they wait for
one that has gone quiet are set by the game, and no launch file changes them. `MapHash` is
not read either: the machines compare the games they have loaded before play begins, which
settles the same question for themselves.

These keys are read but change nothing yet: `IsHost`, `Tournament`, `GameID`,
`WriteStatistics`, the automatic-save scheduling keys, `BuildOffAlly`,
`AttackNeutralUnits`, `ScrapMetal`, `AutoSurrender`, `ContinueWithoutHumans`,
`QuickMatch`, `SkipScoreScreen`, `PlayMoviesInMultiplayer`, `CustomLoadScreen`,
`CustomLoadScreenPos`, and `DifficultyName`.
