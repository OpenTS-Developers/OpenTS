---
title: Campaign progression and carry-over
summary: "Runs the sequence from a campaign mission being won to the next one starting, and carries the global flags, spare money, mission timer, difficulty and stage across the boundary."
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

Winning a campaign mission does not return the player to the menus. The game plays out the mission's endings, decides where the campaign goes, tears the scenario down and loads the next one, all without leaving the running session — and five pieces of state are carried across that boundary by hand, because nothing else survives it. This page owns the sequence and the carried state. What each setting does on its own belongs to that setting's page.

None of this applies to a multiplayer or skirmish session, which stops at its own score screen and shuts the game down.

## Campaigns and stages

A **campaign** is one entry in the `[Battles]` list of `battle.ini`, and it supplies three things the progression needs: the [`Scenario`](/keys/scenario/#scope-campaign) it begins at, the [`CD`](/keys/cd/#scope-campaign) its files are expected on, and the [`FinalMovie`](/keys/finalmovie/) it closes with.

Where a campaign goes after its first mission is not in `battle.ini` at all. It is in `MAPSEL.INI` — `MAPSEL<nn>.INI` for an expansion — which carries one section per HouseType listing that house's **stages** in order under the numeric keys `1` upward. Each stage is a section of its own, naming the scenario it stands for and, under numeric keys of its own, the stages it may lead to. A mission does not remember a filename; it remembers its **stage**, which is a position in that per-house list.

Two consequences run through everything below. Advancing means moving to a stage the current one offers, so a mission can only ever lead somewhere its house's list already reaches. And the list is chosen by the mission's [`Player=`](/keys/player/#scope-scenarios), so which progression a mission sits in follows from the house it is played as.

## The win sequence

A house is not won or lost the instant a trigger says so. The house is flagged, and the flag is acted on only once the borrowed time [`SavourDelay`](/keys/savourdelay/) sets has run out, which is what leaves the player a few moments to watch the last building fall. A win here waits on one condition it does not wait on in a skirmish or multiplayer session: the house's [allow-win blockage count](/mapping/actions/taction-allowwin/) has to have reached zero. The flag simply stands until it does.

Once the flag is acted on, the steps below run in this order, and the whole of it happens inside the running session.

1. The mission's [`Win`](/keys/win/) movie plays.
2. The score screen is presented, unless the mission sets [`SkipScore=yes`](/keys/skipscore/).
3. The [`PostScore`](/keys/postscore/) movie plays, then the [`PreMapSelect`](/keys/premapselect/) movie.
4. [`OneTimeOnly=yes`](/keys/onetimeonly/) stops here and returns to the menus.
5. [`EndOfGame=yes`](/keys/endofgame/) plays the campaign's [`FinalMovie`](/keys/finalmovie/), rolls the credits and returns to the menus.
6. The next mission is chosen — from the map selection screen, or without it when the mission sets [`SkipMapSelect=yes`](/keys/skipmapselect/).
7. The carry-over state is taken from the mission that has just been won.
8. The campaign level number is incremented.
9. The next scenario is loaded, playing its [`Intro`](/keys/intro/), [`Brief`](/keys/brief/) and [`Action`](/keys/action/#scope-scenarios) movies and starting its [`Theme`](/keys/theme/).
10. The carry-over state is applied to the mission now loaded.

Steps 7 and 10 sit either side of the load for a reason. The state is read out of the outgoing mission but the terms on which it is handed over — the share of money, the ceiling on it and whether the timer is inherited — are read out of the **incoming** one, because by step 10 the new scenario file has already been read. A mission reachable by two different routes therefore carries the same share over from either.

:::caution[An allow-win tag holds the victory for good]
The blockage count goes up once for each such tag as the scenario is read, and a tag carrying a single trigger — the ordinary shape — does not bring it back down by firing, so a campaign house carrying one can be flagged to win and never receive the win. [Allow win](/mapping/actions/taction-allowwin/) owns the count, why firing leaves it standing, and the disposal that does clear it.
:::

## What survives the boundary

Five things are copied out of the won mission and put back into the next one. Everything else about the outgoing scenario is discarded with it. The table gives what each one is taken from and what it is put back into; the column to read is the second, because two of the five are not restored to where they came from.

| Carried | Taken from the won mission | Put into the next mission |
| --- | --- | --- |
| The 50 global flags | Their values as the mission ended | Set one at a time, before the first logic frame runs |
| Spare money | The player's available credits | Granted as credits, and added to the house's recorded starting credits |
| The mission timer | Its remaining count | Restored and restarted, and only where [`TimerInherit=yes`](/keys/timerinherit/) and the count is above zero |
| Difficulty | The player's own [difficulty slot](/systems/difficulty/#when-a-house-is-re-handicapped) | Re-applied to the player's house after the money |
| The stage | The stage the advance settled on | Written back over the stage the new scenario reset to zero |

The global flags are the ones a mission sets and clears with [Set global](/mapping/actions/taction-set-global/), and the ones [Global is set](/mapping/events/tevent-global-set/) watches. Their names come from the rules rather than from any one mission, so flag 7 means the same thing in every mission of a campaign. Local variables are not carried and have no equivalent here.

The block is emptied only when a fresh campaign or a fresh single scenario is started from the main menu. It is written into and read back out of save games, so loading a save resumes with whatever the block held at the moment it was written.

## Money

[`CarryOverMoney`](/keys/carryovermoney/) is a fraction of what the player still held when the previous mission was won, clamped to `1` as it is read so that a mission can never hand over more than the player finished with. The product is then compared against [`CarryOverCap`](/keys/carryovercap/) and the smaller of the two is granted, unless the cap is exactly `-1`, which is the one value that means no ceiling. Carrying money forward therefore takes both settings, because [the cap left out](/keys/carryovercap/) is a ceiling of zero.

The grant is not consumed. The block still holds the previous mission's ending balance afterwards, and every path that applies it does so afresh, so a mission replayed after a loss and a mission restarted from the menu are each granted the same money again. The same is true of the inherited timer and the difficulty re-application.

## Choosing the next mission

With `SkipMapSelect` left at its default, winning opens the map selection screen and the player picks one of the stages the current one offers. Cancelling without picking leaves the mission name where it was, so the same mission loads again.

With [`SkipMapSelect=yes`](/keys/skipmapselect/) the mission names its own successor. [`NextScenario`](/keys/nextscenario/) is taken, or [`AltNextScenario`](/keys/altnextscenario/) when the second global flag is set. Either way the name is not loaded directly: the advance clears the current mission name, then compares the name given against the scenario of each stage the current stage offers, ignoring case, and takes the first match.

:::danger[An unreachable name restarts the campaign]
Because the mission name is cleared before the search, a name matching none of the offered stages leaves it empty — and a campaign scenario started with an empty name falls back on the campaign's own first mission. A mistyped or out-of-sequence `NextScenario` therefore drops the player back to the beginning of the campaign, after an error box that says only that map selection could not be started.

Three earlier failures behave differently, because they return before the name is cleared: a `Player=` house with no section in `MAPSEL.INI`, a control file that will not load, and a stage number the list does not reach all leave the current mission's name in place, and the mission the player has just won is loaded again.
:::

Whichever route chose it, the settled stage's own scenario name is uppercased and opened as a file path directly. A name whose file cannot be opened at all shows the game's own message and abandons the load with the mission just won still in memory, because the teardown does not begin until the file has been read. A file that opens and then fails later — a mission demanding an expansion that is not installed, or one whose side or speech archives will not mount — is abandoned after the teardown, and the win sequence goes on to grant the carry-over money through a player house the teardown has already cleared. Neither outcome is guarded: the sequence never asks whether the load succeeded.

## Losing and restarting

Losing plays the mission's [`Lose`](/keys/lose/) movie and offers a replay. Accepting it, and choosing to restart from the menu during a mission, both reload the current mission and then apply the carry-over block to it, exactly as a win does. Neither plays the briefing again: the `Intro`, `Brief` and `Action` movies are shown only on the load that follows a win, and so is the objectives page a mission without a briefing movie opens with.

Abandoning the mission instead sets the campaign level number back to `1` and returns to the menus.

## The campaign level number

The number counts missions started in the current campaign, beginning at `1`, and it is not part of any scenario file. Three things read it. The animated introduction plays only while the number is still `1`, and only for a campaign whose [`CD=`](/keys/cd/#scope-campaign) is below `2`. A house section that omits [`TechLevel=`](/keys/techlevel/#scope-house-per-scenario) is given this number instead, so tech level tracks campaign progress by default. And a score whose [`Scenario=`](/keys/scenario/#scope-themes) is above the current number is refused by the music rotation, which is how a campaign's later music is held back.

The introduction played is `INTR<n>.VQA`, where `n` is the campaign's [`CD=`](/keys/cd/#scope-campaign). Where a deployment holds no such file, `INTRO.VQA` plays instead. Each campaign shipped its own introduction under that one name, on its own disc, so a deployment holding every campaign at once needs the numbered names to reach more than the first.
