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
| `IsHost` | This machine hosts the match against other machines. |

A file that seats more than one person asks for a game against other machines.

## The host

`IsHost=yes` names the machine that hosts a match against other machines. Once the
connections exist it announces itself to every seat, and from then on it is the master: it
hands down the network timings, decides for everyone when the
[game goes out of sync](/systems/out-of-sync-recovery/), and may load a saved game during
play. When it leaves, the lowest seat still held takes over on every machine, and a file that
names no host leaves the first seat in charge from the start.

## Resuming a saved game

`LoadSaveGame=yes` resumes the saved game `SaveGameName` names, and decides the kind of game
on its own: a saved game carries the kind of game it was, the options it was played under
and the houses that played it, so nothing else in the file decides those. A client resuming a
campaign writes little more than the name of the save.

The name is a file inside the game's saved-games folder, and a name written with a path of
its own is reduced to its last part. A save the folder does not hold, or one made by
another version of the game, is refused, and the reason is shown.

A save from a game against other machines resumes as well. Every machine loads its own
copy of the save — the synchronized in-game save writes one on each of them, [numbered
alike](/formats/save-games/#numbered-multiplayer-saves) — while the file seats the same
people again, with the addresses their
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
game's startup movies are skipped. A mission whose briefing movie the game cannot find, or
that names none, shows its written briefing instead, on the same page the objectives button
brings up during play. That page appears only when the mission starts fresh; a restart and
the replay a loss offers go straight to the map.

As the mission begins, the difficulty it is played at is named in a message. The name is the
one `DifficultyName` carries, and otherwise the game's own name for how hard the mission is,
which mirrors `DifficultyModeComputer`: `0` is named Hard, `1` Medium and `2` Easy.
[Difficulty settings](/systems/difficulty/) owns what the pair does.

## The options every house plays under

Read from `[Settings]`: `Bases`, `Credits`, `BridgeDestroy`, `Crates`, `ShortGame`,
`GameSpeed`, `MultiEngineer`, `UnitCount`, `AIPlayers`, `AIDifficulty`, `AlliesAllowed`,
`FogOfWar`, `MCVRedeploy`, `TechLevel`, `Firestorm`, `Seed`, `CoachMode`, and
`PlayMoviesInMultiplayer`.

`CoachMode` decides what a defeated player keeps;
[observers and coach mode](/systems/observers/#coach-mode) owns it.

`PlayMoviesInMultiplayer=yes` plays the scenario's movies in the game the file starts, which a
skirmish or a game against other machines otherwise leaves out. Every machine's file must
carry the same value, as every machine must hold the movies;
[multiplayer movies](/systems/multiplayer-movies/) owns what plays and how the machines skip
a movie together.

A written `Seed` makes a launch repeatable: the same file played twice places every house
the same way. A seed of `0` leaves the placement to chance, which is also what an absent
`Seed` means.

`HarvesterTruce` applies in a game against other machines. A skirmish records it with the
rest of the match's options but does not apply it, exactly as a skirmish set up from the
menu does not.

## Automatic saves

`AutoSaveGame` is the number of frames between automatic saves, and `0` turns them off,
which is also what an absent key means. A written interval replaces the
[`AutoSaveInterval`](/keys/autosaveinterval/) a player's own settings carry, whatever kind
of game the file starts. In a game against other machines every machine saves the same
frame, so the file must carry the same interval on each of them, as a lobby option written
alike into every file does. [Save games](/formats/save-games/#automatic-saves) owns what is
written, under which names, and when.

`NextSPAutoSaveId` and `NextSkirmishAutoSaveId` seed the rotating rings a fresh launch
writes into: a campaign continues from the slot the first names and a skirmish from the
second, both counted from one. A saved game carries its own ring positions, and a resumed
game continues from those rather than from the file. A number below one or beyond the ring
starts the ring at its first slot, so the `-1` a client writes for a loaded save that was no
automatic save is harmless.

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
to the game to choose, which is also what writing no position means. A map hands a seat
whatever it placed for that position by owning it with a
[spawn house](/formats/scenario-objects/#spawn-houses). Alliances are made
exactly as written, before the first frame: a match whose file forbids new alliances still
starts with the ones it wrote. Nothing announces them, and the computer players keep the
alliances the file gave them instead of [closing ranks](/keys/paranoid/) as they do against
a side that allies during play. A seat that watches holds no alliances: its own entries and
any naming it are ignored. [Observers and coach mode](/systems/observers/) owns what a
watching seat is shown and how the match treats it. A map may add alliances of its own
through its spawn house sections, which [`Allies`](/keys/allies/) owns.

A computer player may share the color a person plays; in a game against other machines, two
people may not. The client keys each seat by an order no other machine can rebuild, so two
people of one color would take each other's start position and alliances.

## What a player is shown

These keys change what appears around the match without changing the match itself, so two
machines playing one game need not write them alike.

`SkipScoreScreen=yes` ends a skirmish or network match without its
[score screen](/systems/multiplayer-score-screen/): the round is still counted, and an
ending movie `PlayMoviesInMultiplayer` asked for still plays. The map's own
[`SkipScore`](/keys/skipscore/) is a campaign setting and is not touched by this one.

`CustomLoadScreen` names the picture shown while the scenario loads, in place of the one
the game would have picked for the player's side and screen size. The name is written
whole, extension and all, and is looked for as any other game file is: beside the game, in
the folders a deployment sorts its files into, and inside the archives. A forward slash
separates folders as a backslash does. The picture is a PCX, in 256 colours or in 24-bit
colour, and is centred on the screen. A name no file answers to leaves the game's own
picture in place and says so in the log.

`CustomLoadScreenPos` places the loading bars, as `x,y` within the picture rather than on
the screen, so one position suits every screen size the picture is shown at. Both numbers
must be above zero, and a value the reader cannot make sense of names no position at all,
which leaves the bars where the game puts its own. A picture of the size the game's own
would have been needs no position.

`DifficultyName` names the difficulty in the message a campaign mission opens with.

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
`[Tunnel] Ip` is the loopback address. The tunnel's port must fall between `1` and
`65535`. A version 2 tunnel hands out the numbers it knows the machines by from the
whole signed sixteen-bit range, and the client writes them as they come, so about half
of them are negative; the tunnel matches the same sixteen bits either way, and any
nonzero number within sixteen bits either side of zero is accepted.

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
- a machine other than this one is given no port to answer on, or no address to answer
  at when no tunnel carries the match;
- a `[Tunnel]` section carries no address, or one that names no machine;
- `[Tunnel] Port` falls outside `1` to `65535`;
- through a tunnel, the `Port` the tunnel knows a machine by is zero or beyond sixteen
  bits either side of zero;
- `[Settings] Port` falls outside `1` to `65535` when no tunnel carries the match.

A difficulty easier than the three the game has is not refused: the seat is played as the
easiest opponent the game does have. The two run opposite ways: the easiest opponent plays
at the hardest of the game's three settings.

## What the game does not take from a launch file

The timing keys are not read at all, `ReconnectTimeout` and `ConnTimeout` among them. How
far ahead the machines run, how often they exchange their orders, and how long they wait for
one that has gone quiet are set by the game, and no launch file changes them: a machine
that stops answering brings up the reconnect dialog after seven seconds and is given up
after forty, the waits a LAN game has always used. `MapHash` is
not read either: the machines compare the games they have loaded before play begins, which
settles the same question for themselves.

A client that launches a custom mission writes the mission's own section into the file,
names it with `ReadMissionSection`, and identifies it with `CustomMissionID`. None of the
three is read. The section carries the loading-screen names another game in the series
reads and an identity that game stamps into its saves; the picture a mission wants reaches
this game through `CustomLoadScreen` instead.

These keys are read but change nothing yet: `Tournament`, `GameID`,
`WriteStatistics`, `BuildOffAlly`,
`AttackNeutralUnits`, `ScrapMetal`, `AutoSurrender`, `ContinueWithoutHumans`, and
`QuickMatch`.
