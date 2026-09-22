---
title: Campaign progression and carry-over
summary: "Runs the sequence from winning a campaign mission to starting the next one, and carries the global flags, spare money, mission timer and campaign stage across."
category: maps-scenarios
keys:
  - NextScenario
  - AltNextScenario
  - SkipMapSelect
  - PreMapSelect
  - OneTimeOnly
  - EndOfGame
  - CarryOverMoney
  - CarryOverCap
  - TimerInherit
  - SkipScore
  - PostScore
  - Intro
  - Brief
  - Action
  - Win
  - Lose
  - FinalMovie
  - Theme
  - Player
  - SpeechSide
  - Scenario
  - CD
  - SavourDelay
  - TechLevel
related:
  - type: system
    id: difficulty
  - type: format
    id: theme-ini
  - type: action
    id: TACTION_WIN
  - type: action
    id: TACTION_LOSE
  - type: action
    id: TACTION_ALLOWWIN
  - type: action
    id: TACTION_SET_GLOBAL
  - type: action
    id: TACTION_SET_TIMER
  - type: event
    id: TEVENT_GLOBAL_SET
---

When the player wins a campaign mission, the game plays the mission's ending movies, decides which mission comes next and loads it, without returning to the menus. A few pieces of state are copied out of the won mission and applied to the next one. This page covers that sequence and the carried state. Each setting's page covers what the setting does by itself.

Skirmish and multiplayer games never advance to another mission. Winning or losing one of those ends the game.

## Campaigns and stages

A **campaign** is one entry in the `[Battles]` list of `battle.ini`. It supplies three things the progression uses: the [`Scenario`](/keys/scenario/#scope-campaign) its first mission loads from, the [`FinalMovie`](/keys/finalmovie/) played after its last mission, and a [`CD`](/keys/cd/) number that picks its [introduction movie](#the-campaign-level-number).

The route from one mission to the next is not in `battle.ini`. It is in `MAPSEL.INI`, or `MAPSEL01.INI` for Firestorm missions, which gives each campaign house an ordered list of **stages**. Each stage names the mission it plays in its `Scenario=` key and lists the stages it can lead to. [MAPSEL.INI](/formats/mapsel-ini/) documents the file.

The campaign records its progress as a **stage number**: the current stage's position in the house's list. No mission file sets it. Each advance sets the number to the stage chosen, and a replay, a restart or a loaded save keeps the number the mission had.

Every other mission load starts at the first stage in the list. That includes a new campaign and a campaign mission that a launch file starts partway through a chain. Winning such a mission offers the first stage's choices. With `SkipMapSelect=yes`, its `NextScenario` is matched against those choices, and unless one of them names it, the choice fails as [When the choice fails](#when-the-choice-fails) describes.

Two rules follow from this:

- A mission can lead only to a stage that the current stage lists as a choice.
- The mission's [`Player=`](/keys/player/#scope-scenarios) house selects the list, so the house a mission is played as decides which progression it belongs to.

## The win sequence

A win does not take effect the moment a trigger declares it. The player's house is flagged to win, and the win takes effect when the pause set by [`SavourDelay`](/keys/savourdelay/) has run out, which leaves the player a few moments to watch the last building fall. [Announce Win](/mapping/actions/taction-announce-win/) skips the pause.

In a campaign, the win also waits until the house's [allow-win count](/mapping/actions/taction-allowwin/) is zero. The flag stays set until then. Skirmish and multiplayer games ignore the count.

Once the win takes effect, these steps run in order:

1. The mission's [`Win`](/keys/win/) movie plays.
2. The score screen appears, unless the mission sets [`SkipScore=yes`](/keys/skipscore/).
3. The [`PostScore`](/keys/postscore/) movie plays, then the [`PreMapSelect`](/keys/premapselect/) movie.
4. With [`OneTimeOnly=yes`](/keys/onetimeonly/), the game ends here.
5. With [`EndOfGame=yes`](/keys/endofgame/), the campaign's [`FinalMovie`](/keys/finalmovie/) plays, the credits roll and the game ends.
6. The next mission is chosen, on the map selection screen or, with [`SkipMapSelect=yes`](/keys/skipmapselect/), by the mission itself.
7. The carry-over state is copied from the won mission.
8. The [campaign level number](#the-campaign-level-number) goes up by one.
9. The next mission loads. Its [`Intro`](/keys/intro/), [`Brief`](/keys/brief/) and [`Action`](/keys/action/#scope-scenarios) movies play, and its [`Theme`](/keys/theme/) music starts.
10. The carry-over state is applied to the mission just loaded.

When the game ends, the player returns to the main menu. A game that a client launched exits instead.

The carry-over state comes from the won mission at step 7, but the terms of the handover come from the incoming mission, which has been loaded by step 10. Those terms are the share of money carried, the ceiling on it, and whether the timer is inherited. A mission reachable by two different routes therefore receives the same share from either.

:::caution[An allow-win tag holds the victory for good]
Each tag holding an Allow Win action adds one to its house's count when the scenario loads. A tag with a single trigger, which is the usual shape, does not take its one back when the trigger fires. A campaign house with such a tag can be flagged to win and never receive the win. [Allow Win](/mapping/actions/taction-allowwin/) explains why firing leaves the count in place, and which way of removing the tag does clear it.
:::

## What survives the boundary

The game copies five items out of the won mission and applies four of them to the next one. Everything else about the won mission, including its units, structures and triggers, is discarded.

| Carried | Taken from the won mission | Put into the next mission |
| --- | --- | --- |
| The 50 global flags | Their values when the mission ended | Set on the new mission before its first game frame |
| Spare money | The player's money, including the value of stored Tiberium | A share of it granted as credits, as [Money](#money) describes |
| The mission timer | Its remaining count | Restored and restarted, only if the new mission sets [`TimerInherit=yes`](/keys/timerinherit/) and the count is above zero |
| Difficulty | The player's difficulty slot | Not applied. The new mission [handicaps each house](/systems/difficulty/#when-a-house-is-re-handicapped) as it reads the house |
| The stage | The stage number the advance chose | Replaces the new mission's stage number |

The global flags are the ones [Global Set](/mapping/actions/taction-set-global/) sets and [Global is set](/mapping/events/tevent-global-set/) tests. Their names come from the rules, not from any one mission, so flag 7 means the same thing in every mission of a campaign. Local flags are not carried.

The carried value of global flag `0` also decides where the new mission's view opens: at [`AltHomeCell`](/keys/althomecell/) when the flag is set, and at [`HomeCell`](/keys/homecell/) when it is clear.

The carried state is cleared when the player starts a new campaign or a single scenario from the main menu. A campaign mission started from a launch file also clears it, then sets the global flags [the launch file](/formats/spawn-ini/#a-campaign-mission) lists.

Save games store the carried state, so loading a save restores it as it stood when the save was written.

The incoming mission sets the terms of the handover in its `[Basic]` section, beside the keys that name where the campaign goes next:

```ini title="mission INI"
[Basic]
CarryOverMoney=0.5   ; share of the money the player held when the previous mission was won
CarryOverCap=5000    ; most money the share can grant
TimerInherit=yes     ; resume the previous mission's timer
```

## Money

[`CarryOverMoney`](/keys/carryovermoney/) sets the share of the won mission's money that the next mission grants. A value above `1` counts as `1`, so a mission never grants more than the player finished with. [`CarryOverCap`](/keys/carryovercap/) sets the most the share can grant, except that a cap of exactly `-1` means no ceiling.

Carrying money forward needs both keys. A mission that leaves out `CarryOverCap` has a ceiling of `0`, and the player receives nothing.

In the example above, a player who finished the previous mission with 12,000 credits would have a share of 6,000, and the cap of 5,000 grants 5,000. The score screen's mission efficiency rating counts the grant as money the player started the mission with.

The carried amount is not used up. Each replay or restart of the mission grants the same money again, and an inherited timer is restored again too.

## Choosing the next mission

By default, winning opens the map selection screen, showing the stages the current stage leads to. The player must click one of them; the screen offers no way to leave without choosing. The chosen stage's mission loads next.

With [`SkipMapSelect=yes`](/keys/skipmapselect/), the mission names its successor. The game takes [`NextScenario`](/keys/nextscenario/), or [`AltNextScenario`](/keys/altnextscenario/) when global flag `1` is set. It then compares that name, ignoring case, with the `Scenario=` value of each stage the current stage leads to, and takes the first match. A name that none of those stages lists is not loaded, even when a mission file of that name exists.

### When the choice fails

When the next mission cannot be chosen, the campaign goes on with a mission the player did not pick. Every case below except the last shows an error box saying that map selection could not be started.

| Failure | Route | What loads next |
| --- | --- | --- |
| `NextScenario` or `AltNextScenario` matches none of the stages the current stage leads to | `SkipMapSelect=yes` | The campaign's first mission |
| The `MAPSEL` file is missing or has no stage list for the player's house | Either | The mission just won, again |
| The stage number is beyond the end of the house's list | Either | The mission just won, again |
| The map screen's palette or click map cannot be loaded | Map screen | The mission just won, again |
| The current stage has no `MapVQ=`, so the screen is skipped | Map screen | The mission just won, again |

In every row, the mission that loads is treated as the next mission: its briefing plays, the level number goes up, and it receives the carry-over from the win. When that mission is the one just won, it starts with the flags as they ended and a share of the money the player finished with. The state it received when it was first reached is not applied again.

:::caution[An unreachable name restarts the campaign]
A mistyped or out-of-sequence `NextScenario` sends the player back to the campaign's first mission. The stage number still points at the stage of the mission just won, so winning that first mission again offers the later stage's choices.
:::

### Loading the chosen mission

The chosen stage's `Scenario=` value is opened as the mission file. A load that fails shows an error message saying the scenario could not be read. What follows depends on when it failed:

- If the file cannot be opened, the mission just won stays on screen with the player's input still locked from the win. The carry-over state is applied to it anyway, so the player is granted the money share a second time.
- If the load fails after the file opens, the mission just won has already been cleared. If the new mission requires an expansion that is not installed, or its side or speech files cannot be loaded, the game then crashes when it applies the carry-over state.

## Losing and restarting

Losing plays the mission's [`Lose`](/keys/lose/) movie and asks whether to replay. Replaying, or choosing to restart from the in-game menu, reloads the current mission and applies the carry-over state to it again, as a win does. The mission therefore starts with the same flags, money and timer it received when it was first reached.

A reload skips the briefing: the `Intro`, `Brief` and `Action` movies do not play, and a mission without a briefing movie does not open with its objectives page. The difficulty message appears on every load.

A reload reads the mission file from disk again, so a file edited or replaced since the mission started takes effect. With the deployment's [`CarryScenarioFile=yes`](/formats/opents-ini/#what-a-save-carries), it reads the copy kept when the mission was loaded instead. That copy is what lets a client restart a mission after resuming a [saved game](/formats/save-games/#what-the-file-holds), because the client replaces `spawnmap.ini` with a stub when it resumes one.

Declining the replay ends the game. Abandoning the mission from the in-game menu also ends it, and sets the campaign level number back to `1`.

## The campaign level number

The campaign level number goes up by one each time a campaign mission is won and the next one loads. No scenario file sets it. It is `1` when the game starts, and three other things change it:

- Abandoning a mission from the in-game menu sets it back to `1`.
- Loading a saved game restores the number the save recorded.
- A LAN game sets it to the position of its map in the map list, counting from `0`.

Three things read the number:

- The animated introduction plays when a campaign mission loads with its briefing while the number is `1`, and only for a campaign whose [`CD=`](/keys/cd/) is below `2`. A new campaign's first mission and a mission that a launch file starts partway through a campaign both load with the briefing. A replay or restart skips the briefing, so it never plays the introduction.
- A house section that leaves out [`TechLevel=`](/keys/techlevel/#scope-house-per-scenario) is given this number as its tech level, so by default tech level rises with campaign progress.
- In single-player games, a music track whose [`Scenario=`](/keys/scenario/#scope-themes) is above the current number is skipped whenever the game picks the next track, and is missing from the track list in the sound options. This is how a campaign holds its later music back.

Starting a new campaign does not reset the number, so the campaign inherits whatever value the last game left. A campaign that ends through its final mission, `OneTimeOnly` or a declined replay leaves the number raised. A campaign started next, in the same run of the game, then begins with that number: its animated introduction does not play, and its houses without `TechLevel=` start at the higher tech level. After a LAN game, the number is that map's list position, so the introduction and the default tech level depend on which map was played.

The introduction played is `INTR<n>.VQA`, where `n` is the campaign's [`CD=`](/keys/cd/). If that file is not available, `INTRO.VQA` plays instead. Each original campaign shipped its introduction as `INTRO.VQA` on its own disc, so a deployment holding every campaign needs the numbered names to play more than one of them.
