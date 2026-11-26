---
format_id: launch-request
title: Launch request
summary: Describes the game, saved game or map capture a launcher asks for with -LAUNCH=, and the result file the game writes back.
kind: file
source_files:
- code/launcher.cpp
- code/launcherapply.cpp
- code/queue.cpp
- code/launch/gamelaunch.cpp
- code/gamedirs.cpp
- code/except.cpp
- code/launch/launchmessages.cpp
filenames:
- '*'
related:
- type: command
  id: launch:launch
- type: format
  id: spawn-ini
- type: format
  id: capture-ini
- type: format
  id: stats-dmp
---

A launcher writes a launch request file and starts the game with [`-LAUNCH=<file>`](/using/command-line/#launch). The game reads the file, plays the game, resumes the saved game or makes the map capture it asks for in place of the startup movies and the menu, and exits when that ends. A relative file name is taken from the game's folder. When the request gives a result file, the game writes it before it exits.

## Encoding

The file is a `Request` message of the `launch.proto` schema in the OpenTS repository's `docs/launch` folder, in the Protocol Buffers binary encoding. The game refuses a file that:

- is larger than 1,048,576 bytes, or is not a valid encoding of the message;
- has a `version` other than `1`;
- sets no verb;
- has a text field that is not valid UTF-8.

## What a request asks for

A request sets exactly one verb:

| Verb | What this version does |
| --- | --- |
| `play` | Starts a campaign mission, a skirmish, or a game against other machines. |
| `load` | Resumes a saved game. |
| `capture` | Draws a map to a PNG picture, as the [map capture](/formats/capture-ini/) switches do. |
| `replay` | Refuses the request. |
| `verify` | Refuses the request. |

`request_id` is any text; the result file repeats it. `channel` is not read.

## A play request

`game` is what every machine in the game shares, and must be the same on each of them. `map_path`, `session` and `network` belong to this machine.

| Field | Meaning |
| --- | --- |
| `game.kind` | `1` for a campaign mission, `2` for a skirmish, `3` for a game against other machines. Use `3` exactly when more than one player is human, observers included. |
| `game.seed` | The random seed. A game against other machines needs one. A seed that is set is used as given, `0` included; a campaign mission or skirmish without one draws a seed. |
| `game.players` | At most eight players, human players first. In a skirmish or against other machines a player's position in the list is the house it becomes. |
| `game.options` | The game options, each key at most once, from the table below. |
| `game.anonymous` | Shows players as "Player N" in place of their names. |
| `game.global_flags` | The numbers of the scenario's global flags that are set when a campaign mission starts, from `0` to `49`, ascending, each once. Refused in any other game. |
| `map_path` | The scenario to play, opened as the game opens any file. Required. |
| `session.local_player` | The position in `game.players` of the human playing on this machine; `0` in a campaign mission. Required. |
| `session.host` | Makes this machine the host of a game against other machines, as [`Host=yes`](/formats/spawn-ini/#the-host) does. |
| `session.autosave_interval` | Frames between automatic saves; `0` for none. |
| `session.next_campaign_autosave`, `session.next_skirmish_autosave` | The automatic-save slot to write next, counted from `0`. |
| `session.skip_score_screen` | Passes over the score screen at the end of a game. |
| `session.write_statistics`, `session.game_id`, `session.map_hash` | Write [`stats.dmp`](/formats/stats-dmp/) with this game number and map hash. |
| `session.map_name` | The map's name, which only `stats.dmp` reports. |
| `session.load_screen` | The picture shown while the scenario loads, and where the loading bars go in it. |
| `session.difficulty_name` | Text shown in place of the campaign difficulty's name. |
| `network.port` | This machine's UDP port, or its number on the tunnel when `network.tunnel` is set. |
| `network.peers` | One entry for each other human player: its position in `game.players`, its IPv4 address, and its port or tunnel number. |
| `network.tunnel` | The address and port of a tunnel to play through. |
| `network.connect_timeout`, `network.reconnect_timeout` | The longest wait, in milliseconds, for another machine while the game loads and once it runs. The game rounds down to 16 ms and keeps each wait between 960 and 576,000 ms; `0` keeps the defaults of 57,600 and 38,400 ms. |

Set `network` exactly when `game.kind` is `3`. The game does not read `session.game_mode`, `recording`, or `network.protocol`, `frame_send_rate` and `max_ahead`. A request that sets `fixture` is refused.

A player in `game.players` takes these fields:

| Field | Meaning |
| --- | --- |
| `name` | At most 63 bytes. A human player needs one, and in a game against other machines no two human players have the same name, ignoring case. A computer player without one is shown under a name the game picks. |
| `human` | Set for a human player, unset for a computer player. |
| `observer` | Set for a human player who watches and does not play. |
| `country` | A position in the rules' list of countries, from `0`, or `-1` for one the game picks. |
| `color` | A position in the rules' `[Colors]` list, from `0`, or `-1` for one the game picks. Human players who give a color need different ones. |
| `start` | A start position from `0` to `7`, or `-1` for one the game picks. |
| `ai_level` | A computer player's difficulty: `0` for the hardest, `1`, or `2` for the easiest. Without it the `AIDifficulty` option applies. |
| `allies` | The positions of the players this one is allied with, ascending, never an observer or the player itself. |

A campaign request lists one human player; the mission gives that player a house, so `country`, `color` and `start` are not read.

An option is a key with a number or a flag. A key that does not apply to the kind of game is refused, and so is a value outside the range.

| Key | Value | Games | Accepted | Without it |
| --- | --- | --- | --- | --- |
| `AIDifficulty` | number | skirmish, other machines | `0` (easiest) to `2` (hardest) | `1` |
| `AlliesAllowed` | flag | skirmish, other machines | | no |
| `AttackNeutralUnits` | flag | skirmish, other machines | | no |
| `AutoDeployMCV` | flag | skirmish, other machines | | no |
| `Bases` | flag | skirmish, other machines | | yes |
| `BridgeDestruction` | flag | skirmish, other machines | | yes |
| `BuildOffAlly` | flag | skirmish, other machines | | no |
| `Campaign` | number | campaign | `-1` to the last campaign | `-1` |
| `CampaignComputerDifficulty` | number | campaign | `0` to `2` | `1` |
| `CampaignPlayerDifficulty` | number | campaign | `0` to `2` | `1` |
| `CoachMode` | flag | skirmish, other machines | | no |
| `Crates` | flag | skirmish, other machines | | no |
| `Credits` | number | skirmish, other machines | any | `10000` |
| `Firestorm` | flag | every game | | yes |
| `FogOfWar` | flag | skirmish, other machines | | no |
| `GameSpeed` | number | every game | `0` to `6` | `0` |
| `HarvesterTruce` | flag | skirmish, other machines | | no |
| `MCVRedeploy` | flag | skirmish, other machines | | yes |
| `MultiEngineer` | flag | skirmish, other machines | | no |
| `PlayMovies` | flag | skirmish, other machines | | no |
| `ScrapMetal` | flag | skirmish, other machines | | no |
| `SharedControl` | flag | skirmish, other machines | | no |
| `ShortGame` | flag | skirmish, other machines | | no |
| `SurrenderOnDisconnect` | flag | skirmish, other machines | | yes |
| `TechLevel` | number | skirmish, other machines | any | `10` |
| `UnitCount` | number | skirmish, other machines | any | `0` |

Each key has the meaning of the [`SPAWN.INI`](/formats/spawn-ini/) key of the same name, except these six, which `SPAWN.INI` names differently:

| Key | `SPAWN.INI` key |
| --- | --- |
| `BridgeDestruction` | `BridgeDestroy` |
| `Campaign` | `CampaignID` |
| `CampaignComputerDifficulty` | `DifficultyModeComputer` |
| `CampaignPlayerDifficulty` | `DifficultyModeHuman` |
| `PlayMovies` | `PlayMoviesInMultiplayer` |
| `SurrenderOnDisconnect` | `AutoSurrender` |

`AIDifficulty` numbers difficulties the other way from `ai_level`.

### Countries and colors left to the game

For a `country` or `color` of `-1`, the game draws a value before the map loads. It draws a country from those with [`Multiplay=yes`](/keys/multiplay/), or takes the first country when none has it. It draws a color from the eight colors the menu offers, the `[Colors]` entries numbered `1`, `10`, `13`, `16`, `19`, `23`, `27` and `36`, leaving out every color a player gives and every color an earlier draw took. When all eight are taken, it draws from the other `[Colors]` entries not taken, and when every entry is taken, from all of them. A color a computer player gives is used even when another player gives the same one.

The game draws for the human players in list order, then for the computer players in list order, from random numbers that start from `game.seed`. Every machine in a game therefore draws the same values, and a skirmish without a seed draws differently at each launch.

## A load request

A load request resumes a saved game, as [`LoadSaveGame=yes`](/formats/spawn-ini/#resuming-a-saved-game) does in `SPAWN.INI`. The save gives the kind of game, its scenario, its options and its houses.

| Field | Meaning |
| --- | --- |
| `save` | The file name of the save in the saved-games folder, with no folder part. Required. |
| `session` | The same fields as in a play request. `local_player` and `host` are read only for a game against other machines. `load_screen` and `difficulty_name` take effect only when the scenario is restarted. |
| `network` | Required for a save from a game against other machines, as in a play request, and not read otherwise. |
| `players` | Required for a save from a game against other machines, and not read otherwise. |
| `game_speed` | The speed, `0` to `6`, to play at. Without it the game plays at the saved speed. |
| `anonymous` | Shows players as "Player N" in place of their names, as `game.anonymous` does. A save does not keep it. |

In a game against other machines, each machine loads the save it wrote itself, and `players` lists the human players who rejoin, the same list on every machine. `session.local_player` is the position of the player on this machine, and each entry of `network.peers` gives another position in the list. Each player needs a `name` of 1 to 63 bytes that no other player in the list has, ignoring case, and a `country` and `color` that are `-1` or ones the rules have.

After loading the save, the game gives each player the saved human house with the same name, ignoring case. The houses keep the countries and colors the save gives them. The resume fails when:

- a player has no saved house of that name;
- a player watches in the request but played in the save, or the reverse;
- this machine's save was written by a player other than `session.local_player`.

A human player who played in the save but is not listed keeps the house, and the computer plays it.

A resumed game against other machines first runs at the speed the match was playing at when it was saved. The player with the lowest-numbered house, observers left out, then changes the speed for every machine to the `game_speed` in that player's request. The `game_speed` in the other players' requests is not used.

## A capture request

A capture request sets `map_path` and `out_path` together, as [`-CAPTUREMAP=`](/using/command-line/#capture-map) and [`-CAPTUREOUT=`](/using/command-line/#capture-output) do, or `description_path` alone, as [`-CAPTURE=`](/using/command-line/#capture) does. A relative `out_path` is placed like the result file. A relative `description_path` is taken from the game's folder.

## When the game refuses a request

Besides a file it cannot read, the game refuses:

- `-LAUNCH=` together with `-SPAWN`, [`-CAPTUREMAP=`](/using/command-line/#capture-map), [`-CAPTUREOUT=`](/using/command-line/#capture-output), [`-CAPTURE=`](/using/command-line/#capture), [`-TIME=`](/using/command-line/#tournament-time) or `-SEED`, in any build;
- a `replay` or `verify` request, and a `play` request that sets `fixture`;
- a capture request that sets `description_path` with `map_path` or `out_path`, or that lacks one of `map_path` and `out_path` without `description_path`;
- a play request that breaks a rule on this page, such as a country or color the rules do not have;
- a load request that breaks a rule on this page, or whose save is missing, unreadable, made by another version of the game, or from a game the menu arranged over the local network.

A refused request starts nothing. With a result file, the game reports the refusal there and shows no message. Without one, it shows the reason in a message box, except when a capture switch was also given: that run shows no dialog and writes the reason to the debug log and to the console it was started from.

## The result file

`result_path` is the result file's path. A relative path goes into the [`-USERDIR=`](/using/command-line/#user-directory) folder, or beside the game without one, as `stats.dmp` does; an absolute path is used as given. Once the game has read the request it deletes any file at that path. It writes the result when it exits, or when it crashes after accepting the request, to a temporary file that it then renames over the path, so a launcher never reads part of one. No file after the game exits means it could not read the request or write the file, or that it was stopped before its [crash handler](/using/crash-reports/) wrote the report.

The result is a `Result` message in the same encoding:

| Field | Meaning |
| --- | --- |
| `request_id` | The request's `request_id`. |
| `code` | `1` when the game or capture ran, `2` when the request was refused, `3` when it was accepted and could not be done, such as a capture that failed, a network that did not open, a save that did not load or whose houses do not match `players`, or a scenario that never started, `4` when the game crashed, and `7` when a game against other machines ended at its start because the machines had not loaded the same game, as [Games that differ at the start](/systems/out-of-sync-recovery/#games-that-differ-at-the-start) describes. |
| `message` | Why, in English, when `code` is not `1`. |
| `scenarios` | One entry for each scenario a play or load request started, in order. A resumed save is the first entry. A campaign can start several missions, and a restart starts the same one again. |
| `output_path` | The picture a capture wrote, as an absolute path. |
| `crash_path` | After a crash, the crash report's folder. |

Each scenario entry gives:

| Field | Meaning |
| --- | --- |
| `map_path` | The scenario's file name, in upper case. |
| `outcome` | How the scenario ended, from the table below. |
| `completed` | Set when the game decided the scenario as a win or a loss. |
| `last_frame` | The game's frame counter when the scenario ended. Absent for the scenario a crash ended, and for outcome `7`. |
| `not_recorded` | Always `1`, since this version records no replays; `replay_path` is always empty. |

| `outcome` | When |
| --- | --- |
| `0` | The local player is an observer, and the scenario ended in a way that gives a player `1`, `2` or `4`. Also the scenario a crash ended. |
| `1` | The local player won. |
| `2` | The local player lost. |
| `3` | Not reported by this version. |
| `4` | The scenario was restarted, the player left it, or it ended without being decided, such as when the player pressed Cancel in the [reconnect dialog](/systems/reconnect-dialog/) during play or a [multiplayer load](/systems/out-of-sync-recovery/#loading-a-saved-game) failed on this machine. |
| `5` | The game ended because the machines' games differed: the player quit the [out-of-sync dialog](/systems/out-of-sync-recovery/#quitting), the game stopped with `Packet received too late!`, or the machines had not loaded the same game at a multiplayer load. |
| `6` | Every other player voted this machine out during play, or another machine did not answer when play resumed after a multiplayer load. |
| `7` | The game against other machines ended at its start, before its first frame: another machine did not answer, the player pressed Cancel or was voted out while waiting, or the machines had not loaded the same game. |

A machine that stops answering during play is dropped, and the others play on, as [Leaving a match](/systems/leaving-a-match/) describes; the same happens to a player dropped on the loading screen. The players who stay get the outcome of the rest of the game. A machine whose own connection fails cannot tell that apart from the others falling silent, so it drops them in the same way and plays on alone. After the master [continues](/systems/out-of-sync-recovery/#continuing) an out-of-sync game, each machine's outcome likewise comes from the game it plays on.
