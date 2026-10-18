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

A client that sets up matches outside the game, such as the CnCNet client, writes this file beside the game and starts the game with [`-SPAWN`](/using/command-line/spawn). The game then plays the match the file describes instead of showing its menu, and exits when that match ends.

Every key on this page belongs in the `[Settings]` section unless the page names another section. The game ignores keys it does not know. Values are read as follows:

- A `yes` or `no` value is read from its first character, in upper or lower case: `y`, `t` or `1` means yes, and `n`, `f` or `0` means no. Any other value, such as `on`, leaves the key at its default.
- Write numbers in decimal, with a leading `-` for a negative one. A number key given text does not fall back to its default: `Credits=lots` reads as `0`.

The game checks the file once, when it starts. [When something is wrong](#when-something-is-wrong) lists what refuses a launch.

## What the file asks for

The `[Settings]` section says what kind of game to start.

| Key | Meaning |
| --- | --- |
| `Scenario` | The scenario file to play. Defaults to `spawnmap.ini`. |
| `IsSinglePlayer` | `yes` plays a campaign mission rather than a match. |
| `LoadSaveGame`, `SaveGameName` | `LoadSaveGame=yes` resumes the saved game named by `SaveGameName`. |
| `Host` | `yes` marks this machine as the host of a game against other machines. [The host](#the-host) says what it changes. |

`IsSinglePlayer`, `LoadSaveGame` and `Host` default to `no`. The game starts the first kind of game in this list that the file asks for:

1. `LoadSaveGame=yes` [resumes a saved game](#resuming-a-saved-game).
2. `IsSinglePlayer=yes` plays a [campaign mission](#a-campaign-mission).
3. A file that seats more than one person starts a [game against other machines](#a-game-against-other-machines).
4. Any other file starts a skirmish.

[Who is playing](#who-is-playing) describes how the file seats people.

## The host

The [master](/systems/out-of-sync-recovery/#the-master) of a game against other machines sets the network timing, decides for everyone when the game goes out of sync, and can load a saved game during play. When the match starts, the master is the lowest seat in [house order](#who-is-playing), the person with the lowest `Color`, on every machine. When the master leaves, the lowest seat still held takes over.

`Host=yes` marks this machine as the host, in any seat. The host becomes the master after an in-game load, if it is still playing, whichever seat it holds. Until then it is the master only when it holds the lowest seat.

## Resuming a saved game

`LoadSaveGame=yes` resumes the save named by `SaveGameName`, from the game's saved-games folder. Write the file name alone; a path written before it is ignored.

The save supplies the kind of game, its scenario, its options and its houses, so `IsSinglePlayer`, `Scenario` and the [match options](#the-options-every-house-plays-under) are not used. The file still sets `GameSpeed`, `PlayMoviesInMultiplayer`, the [automatic-save interval](#automatic-saves), the two [waits](#a-game-against-other-machines) for other machines, `SkipScoreScreen` and `QuickMatch`. `CustomLoadScreen`, `CustomLoadScreenPos` and `DifficultyName` take effect only when the mission is restarted, because loading a save shows no loading picture and no difficulty message. The computer players keep the names the save holds, so `DifficultyBasedAINames` has no effect.

A restart after the resume replays the scenario the save names, or the [copy of it](/formats/save-games/#what-the-file-holds) the save carries when `CarryScenarioFile` in `OPENTS.INI` asks saves to include one.

The launch is refused when `SaveGameName` is empty, or when the save is missing, unreadable, made by another version of the game, or from a game the menu arranged over the local network.

A game against other machines resumes the same way, and each machine loads the save it wrote itself. A save made during such a game writes one file on every machine, [numbered alike](/formats/save-games/#numbered-multiplayer-saves). Each machine's file seats the people again under the names they played with, at the addresses their machines use now.

The file must pass the same checks as a [new match](#when-something-is-wrong), and the launch is also refused when:

- a person the file seats has no house of that name in the save;
- a person watches in the file but played in the save, or the reverse;
- this machine's save was written by another player.

A person who played in the save but is left out of the file keeps their house, and the computer plays it. Before play resumes, the machines compare the games they loaded and stop with `Scenarios don't match.` if they differ.

## A campaign mission

`IsSinglePlayer=yes` plays the mission `Scenario` names. `CampaignID` names the campaign the mission belongs to, counted from `0` in the order the battle files declare the campaigns. `-1`, the default, plays the mission outside any campaign. The campaign decides what follows the mission and which of the game's own introduction and ending movies play, as it does when the campaign is chosen from the menu.

`DifficultyModeHuman` sets the [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot) of the player's houses, and `DifficultyModeComputer` the slot of the computer's houses. Each takes `0` to `2` and defaults to `1`. Slot `0` takes its handicap from the rules' `[Easy]` section, slot `1` from `[Normal]` and slot `2` from `[Difficult]`. [Difficulty settings](/systems/difficulty/) owns what the slots do.

The two difficulty keys are independent, so the file can pair any player difficulty with any computer difficulty. The menu's Hard setting is `DifficultyModeHuman=2` with `DifficultyModeComputer=0`. A restart or the next mission keeps the pair.

`[GlobalFlags]` sets the scenario flags a mission chain carries forward. Each entry, `GlobalFlag0` to `GlobalFlag49`, is `yes` or `no`. The flags are set as the mission starts, so a mission launched partway through a chain begins in the state the earlier missions left.

The game's startup movies are skipped, but the mission's own movies and [briefing](/keys/brief/) play as they do from the menu.

As the mission begins, a message names its difficulty. The name is `DifficultyName` when the file sets it. Otherwise it is Hard, Medium or Easy for a `DifficultyModeComputer` of `0`, `1` or `2`.

Of the [match options](#the-options-every-house-plays-under), a campaign mission uses only `GameSpeed`, `Firestorm` and `Seed`.

## The options every house plays under

These `[Settings]` keys set the rules of a skirmish or a game against other machines. In a game against other machines, write them the same in every machine's file. `BuildOffAlly` is the one exception, as described below.

| Key | Value | When omitted |
| --- | --- | --- |
| `Bases` | `yes` or `no` | `yes` |
| `Credits` | A number | `10000` |
| `BridgeDestroy` | `yes` or `no` | `yes` |
| `Crates` | `yes` or `no` | `no` |
| `ShortGame` | `yes` or `no` | `no` |
| `GameSpeed` | `0`, the fastest, to `6`, the slowest | `0` |
| `MultiEngineer` | `yes` or `no` | `no` |
| `UnitCount` | A number | `0` |
| `AIPlayers` | The number of computer players, from `0` to 8 minus the number of people | `0` |
| `AIDifficulty` | The computer players' difficulty as the player sees it: `0` easy, `1` normal, `2` hard | `1` |
| `AlliesAllowed` | `yes` or `no` | `no` |
| `HarvesterTruce` | `yes` or `no` | `no` |
| `FogOfWar` | `yes` or `no` | `no` |
| `MCVRedeploy` | `yes` or `no` | `yes` |
| `AutoDeployMCV` | `yes` or `no` | `no` |
| `TechLevel` | A number | `10` |
| `Firestorm` | `yes` or `no` | `yes` |
| `Seed` | A number | `0` |
| `CoachMode` | `yes` or `no` | `no` |
| `AutoSurrender` | `yes` or `no` | `yes` |
| `BuildOffAlly` | `yes` or `no` | `no` |
| `AttackNeutralUnits` | `yes` or `no` | `no` |
| `ScrapMetal` | `yes` or `no` | `no` |
| `PlayMoviesInMultiplayer` | `yes` or `no` | `no` |

A computer player plays at the slot opposite `AIDifficulty`, as [Difficulty settings](/systems/difficulty/#from-the-setting-to-a-slot) describes, unless its seat sets its own difficulty under [Who is playing](#who-is-playing).

`CoachMode` decides what a defeated player keeps; [observers and coach mode](/systems/observers/#coach-mode) owns it.

`AutoSurrender=yes` destroys the base of a player who leaves, and `AutoSurrender=no` hands it to the computer instead; [leaving a match](/systems/leaving-a-match/) owns it. Each machine applies its own file's value, so machines whose files disagree fall out of step as soon as somebody leaves.

`AttackNeutralUnits=yes` lets a target scan consider a neutral house, which a match otherwise passes over; [target selection](/systems/target-selection/#why-a-candidate-is-rejected) owns what is then picked. Each machine scans for itself, so every file needs the same value.

`ScrapMetal=yes` makes a wreck leave the animations its type gives [`ScrapExplosion`](/keys/scrapexplosion/) instead of [`Explosion`](/keys/explosion/). The animation is drawn with the shared random number generator, so every file needs the same value.

`AutoDeployMCV=yes` deploys every house's starting base unit as the match opens; [starting forces](/systems/starting-forces/#the-base-unit) owns what that leaves on the map.

`BuildOffAlly=yes` lets a player place buildings against a mutually allied house's base as well as their own; [base placement and adjacency](/systems/base-adjacency/#building-off-an-ally) owns what counts as an anchor. The game tests it only on the machine that places the building, so a file that disagrees with the others only changes where its own player can build.

`PlayMoviesInMultiplayer=yes` plays the scenario's movies, which a skirmish or a game against other machines otherwise leaves out. Every machine must have the same value and hold the movies; [multiplayer movies](/systems/multiplayer-movies/) owns what plays and how the machines skip a movie together.

A nonzero `Seed` makes a launch repeatable: launching the same file again places every house the same way, and builds the same map from a scenario file with [`RandomMap=yes`](/keys/randommap/). In a skirmish or a campaign mission, `0` draws a new seed at each launch. In a [game against other machines](#a-game-against-other-machines), every machine uses the seed exactly as written, `0` included, so write a new seed for each match.

`HarvesterTruce=yes` turns on the [harvester truce](/keys/harvesterimmune/) in a skirmish and in a game against other machines.

## Automatic saves

`AutoSaveGame` is the number of frames between automatic saves. `0`, the default, or a negative number turns them off. The file's value replaces the player's own [`AutoSaveInterval`](/keys/autosaveinterval/) in every kind of game the file starts, resumed games included. In a game against other machines, write the same interval in every file so that every machine saves at the same frame. [Save games](/formats/save-games/#automatic-saves) owns what is written, under which names, and when.

`NextSPAutoSaveId` sets the next slot of the campaign's rotating automatic saves, and `NextSkirmishAutoSaveId` the next slot of the skirmish's, each from `1` to `5`. An omitted key, or any other value such as `-1`, starts that rotation at slot `1`. A resumed game continues from the slots its save holds instead.

## Who is playing

Each person has a section: `[Settings]` for the player at this machine, and `[Other1]` to `[Other7]` for the other people. Each section gives the person's `Name`, `Side` and `Color`:

- `Name` is UTF-8 text. A name longer than 63 bytes is shortened.
- `Side` is the person's country, numbered from `0` in the order of the rules' `[Houses]` list.
- `Color` is the person's color, numbered from `0` in the order of the rules' `[Colors]` list. An entry that repeats an earlier name is skipped and takes no number. In a game against other machines, no two people may share a color.

A person's section must give a valid `Side` and `Color`; it cannot leave them to the game.

The file names each seat in one of two ways, both written `Multi1` to `Multi8`:

- By file position: `[Settings]` is `Multi1`, `[Other1]` is `Multi2`, and so on to `[Other7]` as `Multi8`.
- By house order: the people first, from the lowest `Color` up, then the computer players in file position order. This is the order in which the game creates the houses.

When the people fill `[Settings]` and `[Other1]` onward without gaps, each computer player has the same number both ways, and only the people's numbers can differ.

The file positions that no person's section claims are for computer players. The first `AIPlayers` of them, lowest first, each hold one computer player, and the rest stay empty. A computer seat is described under its file position:

| Section | Entry | Meaning |
| --- | --- | --- |
| `[HouseColors]` | `Multi1`–`Multi8` | The color that seat plays, numbered as a person's `Color`, or `-1` for the game's choice. |
| `[HouseCountries]` | `Multi1`–`Multi8` | The country that seat plays, or `-1` for the game's choice. |
| `[HouseHandicaps]` | `Multi1`–`Multi8` | The difficulty slot that seat plays at, or `-1` to follow `AIDifficulty`. |

An entry omitted from these three sections means `-1`, and an entry at a person's file position is ignored.

An entry of `-1` leaves the choice to the game, as in a game set up from the menu. The game picks a country the rules offer in multiplayer, and one of the eight colors the menu offers that no person or earlier computer player has taken. A color the file writes is used as given, so a computer player can share a color with a person or with another computer player.

A handicap sets the seat's [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot) directly: `0` is the hardest opponent and `2` the easiest, and `3` to `6` play as `2`. This runs opposite to `AIDifficulty`: a handicap of `0` and `AIDifficulty=2` both make a hard opponent. A computer player at slot `0` reads the rules' `[Easy]` section and is the opponent a player meets on Hard.

These sections name a seat by house order:

| Section | Entry | Meaning |
| --- | --- | --- |
| `[SpawnLocations]` | `Multi1`–`Multi8` | The map start position, `0` to `7`, that seat begins at, or `-1` for the game's choice. |
| `[Multi1_Alliances]`–`[Multi8_Alliances]` | `HouseAllyOne`–`HouseAllyEight` | Each entry names one seat this seat allies with, by house order counted from `0`, so `0` is `Multi1`. `-1` names none. |
| `[IsSpectator]` | `Multi1`–`Multi8` | `yes` makes the person in that seat watch instead of play. A computer player's seat ignores it. |

The two orders differ whenever the people's colors do not rise in file position order. In this example, which leaves out the match options and connection keys, Boris writes himself first, but Alice's lower color puts her first in house order. One computer player is allied with Boris.

```ini title="SPAWN.INI on Boris's machine"
[Settings]
Name=Boris
Side=1
Color=1           ; file position Multi1, house order Multi2
AIPlayers=1

[Other1]
Name=Alice
Side=0
Color=0           ; file position Multi2, house order Multi1

[HouseColors]
Multi3=2          ; the computer player, at the first free file position

[SpawnLocations]
Multi1=0          ; Alice
Multi2=1          ; Boris
Multi3=2          ; the computer player

[Multi2_Alliances]
HouseAllyOne=2    ; Boris allies with the computer player

[Multi3_Alliances]
HouseAllyOne=1    ; and the computer player with Boris
```

A seat with no start position, or one outside `0` to `7`, gets one from the game; [starting forces](/systems/starting-forces/#the-start-position) owns how positions are assigned, including a position the map does not declare or another seat already holds. A map hands a seat whatever it placed for that position by owning it with a [spawn house](/formats/scenario-objects/#spawn-houses).

An alliance entry works one way: to make two seats allies, list each in the other's section, as the example does.

The file's alliances are made before the first frame. `AlliesAllowed=no` does not prevent them; it only stops players from changing alliances during play. No message announces the file's alliances, and the computer players keep them instead of [closing ranks](/keys/paranoid/) as they do against a side that allies during play.

A seat that watches holds no alliances: its own entries and any entries naming it are ignored. [Observers and coach mode](/systems/observers/) owns what a watching seat is shown and how the match treats it.

A map may add alliances of its own through its spawn house sections, which [`Allies`](/keys/allies/) owns.

## What a player is shown

These keys change what appears around the match without changing the match itself, so two machines playing one game need not write them alike.

`SkipScoreScreen=yes` ends a skirmish or network match without its [score screen](/systems/multiplayer-score-screen/). An ending movie that `PlayMoviesInMultiplayer` asked for still plays. The map's own [`SkipScore`](/keys/skipscore/) is a campaign setting and is not affected.

`CustomLoadScreen` names the picture shown while the scenario loads, in place of the one the game picks for the player's side and screen size. Write the file name with its extension. The game looks for it as it looks for any game file: beside the game, in the folders a deployment sorts its files into, and inside the archives. A forward slash separates folders as a backslash does. The picture is a PCX in 256 colors or 24-bit color, centered on the screen. If no file has that name, the game's own picture stays and the log says so.

`CustomLoadScreenPos` places the loading bars at `x,y` within the picture rather than on the screen, so one position suits every screen size. Both numbers must be above `0`. Otherwise, or when the value is not two numbers, the bars stay where the game puts them for its own picture, which suits a picture of the same size.

A saved game keeps the picture and the bar position. The first restart after the save is loaded shows them again, whether a launch file or the game's load menu loaded it. When the launch file that started the current session names a picture, every restart shows that picture instead. Otherwise the kept picture lasts only for that first restart: later restarts show the game's own picture, and a save made after that restart keeps none.

`DifficultyName` replaces the difficulty name in the message a [campaign mission](#a-campaign-mission) opens with. Only its first 31 bytes are shown.

`DifficultyBasedAINames=yes` names each computer player `Hard AI`, `Medium AI` or `Easy AI` instead of `Computer`. The name follows the seat's `[HouseHandicaps]` entry: `0` is Hard, `1` Medium, and `2` to `6` Easy. A seat without an entry is named after `AIDifficulty`, so `AIDifficulty=2` names it Hard. The name does not reflect the [bonus](/systems/difficulty/#the-computers-bonus-with-more-than-one-human) a computer player can get in a match with more than one person.

`QuickMatch=yes` shows every player, computer players included, as `Player 1` to `Player 8` instead of by name. This covers the loading screen, the radar's name list, chat, the messages about alliances, defeats, changed settings and players leaving, the reconnect and out-of-sync dialogs, the vote to skip a movie, the score screen, and the [`-MPDEBUG`](/using/command-line/multiplayer-debug/) overlay. A player's number is their seat's place in [house order](#who-is-playing), so it is the same on every machine and does not change when others leave. The real names stay in this file and still appear in the game's log.

## A game against other machines

Each machine writes its own file, with itself in `[Settings]` and the other people in the `[OtherN]` sections. Write the other people in consecutive sections from `[Other1]`, so the computer players take the same file positions in every file. Apart from where each person is written, `Host`, and the `Ip` and `Port` entries of `[Settings]` and `[OtherN]`, every file must describe the same match:

- the same `Scenario`, and the same scenario file on every machine;
- the same [match options](#the-options-every-house-plays-under) and `AutoSaveGame`;
- the same people, with the same `Name`, `Side` and `Color`;
- the same entries in `[HouseColors]`, `[HouseCountries]`, `[HouseHandicaps]`, `[SpawnLocations]`, the `[MultiN_Alliances]` sections and `[IsSpectator]`;
- the same `[Tunnel]` section, when the match is played through a tunnel.

The keys under [What a player is shown](#what-a-player-is-shown) and the two waits below may differ between machines.

Before play begins, the machines compare the scenarios they loaded. If they differ, the match stops with the message `Scenarios don't match.`

The `Session identity:` line in an [out-of-sync report](/using/out-of-sync-reports/) differs between machines whose files disagree on anything in the list above, except `Name`, `AutoSaveGame` and the `[Tunnel]` section. Scenario files that differ under the same name give the same identity. `Host`, the `Ip` and `Port` entries, the waits and the keys under [What a player is shown](#what-a-player-is-shown) do not change it.

Without a tunnel, each `[OtherN]` section also gives `Ip`, the address that machine answers on, and `Port`, the port it listens on. `[Settings] Port` is the port this machine listens on, `1234` by default. Write each address as a dotted IPv4 address such as `203.0.113.7`; a host name is refused.

A `[Tunnel]` section with a nonzero `Port` plays the match through the tunnel server at its `Ip` and `Port`, and the `Ip` entries of `[Settings]` and `[OtherN]` are ignored. For a tunnel server on the same computer, write `127.0.0.1` as `[Tunnel] Ip`.

Through a tunnel, each `Port` entry of `[Settings]` and `[OtherN]` holds a tunnel number instead of a port. `[Settings] Port` is the number the tunnel knows this machine by, and each `[OtherN] Port` the number it knows that machine by, so the numbers trade places between the files. Write the numbers as the tunnel gives them, negative ones included: any nonzero number from `-65535` to `65535` is accepted. The numbers have no default, so an omitted one refuses the launch.

`ConnTimeout` and `ReconnectTimeout` set how long this machine waits for another, in ticks of 16 milliseconds, 62.5 to the second. A value outside 60 to 36000 ticks, 0.96 seconds to 9.6 minutes, is moved to the nearer limit.

`ConnTimeout`, 3600 ticks (about 58 seconds) by default, is how long loading may go without progress. The wait starts over whenever loading progresses. When it runs out, this machine drops every machine that has not finished loading.

`ReconnectTimeout`, 2400 ticks (about 38 seconds) by default, is how long a machine may go quiet during play. The [reconnect dialog](/systems/reconnect-dialog/) counts it down.

Each machine applies its own waits, so files that differ do not put the match out of step. Writing the same values everywhere is still the better choice, since the machine with the shortest wait is the first to drop a slow machine.

## When something is wrong

The game refuses a launch it cannot play. It shows the reason, writes it to the log, and exits instead of falling back to its menu.

Any launch is refused when `SPAWN.INI` is missing or `GameSpeed` is outside `0` to `6`.

A campaign mission is also refused when `DifficultyModeHuman` or `DifficultyModeComputer` is outside `0` to `2`, or when `CampaignID` is below `-1` or names a campaign the game does not have.

A resumed game is also refused for the reasons under [Resuming a saved game](#resuming-a-saved-game).

A new skirmish, and any game against other machines, new or resumed, is also refused when:

- the file has no `[Settings]` or `[OtherN]` section, so it seats no person;
- `AIDifficulty` is outside `0` to `2`;
- `AIPlayers` is negative or more than the seats no person holds;
- a person's `Side` is not a country the rules have, or their `Color` is not an entry in the rules' `[Colors]` list; a computer seat's country and color are checked the same way unless they are `-1`;
- a `[HouseHandicaps]` entry is outside `-1` to `6`;
- an occupied seat's alliance entry is outside `-1` to `7`, or names a seat that is empty;
- every person watches and no computer plays; this check does not apply to a resumed game.

A game against other machines, new or resumed, is also refused when:

- a person has no `Name`, or two people have the same name, ignoring case, or the same `Color`;
- without a tunnel, `[Settings] Port` or another person's `Port` is outside `1` to `65535`, or another person's `Ip` is not a dotted IPv4 address other than `0.0.0.0`;
- through a tunnel, `[Tunnel] Port` is negative or above `65535`, or `[Tunnel] Ip` is not a dotted IPv4 address other than `0.0.0.0`;
- through a tunnel, `[Settings] Port` or another person's `Port` is `0` or outside `-65535` to `65535`;
- the network cannot be opened.

## What the game does not take from a launch file

The game sets how far ahead the machines run and how often they exchange their orders, and no launch file changes them. `MapHash` is not read either; the machines compare the scenarios they loaded instead.

`AimableSams` is not read: a defense whose weapon reaches only the air can be aimed at a chosen aircraft, which [`SAM`](/keys/sam/) owns.

`ContinueWithoutHumans` is not read. A match ends once no person is left playing it, except a match in which every person is [watching](/systems/observers/), which runs on until one side remains.

`ReadMissionSection` and `CustomMissionID` are not read, and neither is the mission section a client writes for a custom mission. A custom mission's loading picture reaches this game through `CustomLoadScreen`.

`UIMapName` is not shown anywhere.

`Tournament`, `GameID` and `WriteStatistics` are not read.
