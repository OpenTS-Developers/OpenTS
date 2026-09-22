---
title: Difficulty settings and handicaps
summary: "Turns the chosen difficulty into one slot per house and scales that house's damage, speed, armor, rate of fire, prices and build times by the figures the slot holds."
category: ai-teams
keys:
  - AIHateDelays
  - Armor
  - BuildTime
  - CompEasyBonus
  - Cost
  - DestroyWalls
  - Difficulty
  - FillEarliestTeamProbability
  - FirePower
  - Firepower
  - GameSpeedBias
  - Groundspeed
  - MaximumAIDefensiveTeams
  - MinimumAIDefensiveTeams
  - MultiplayerAICM
  - PlayerControl
  - ROF
  - RepairDelay
  - TeamDelays
  - TotalAITeamCap
related:
  - type: system
    id: production
  - type: system
    id: repair
  - type: system
    id: ai-team-production
  - type: system
    id: ai-base-building
  - type: system
    id: target-selection
  - type: system
    id: trigger-springing
---

## From the setting to a slot

Every house plays at a difficulty slot of 0, 1 or 2. The slot selects the rules section the house takes its handicap from: `[Easy]` for slot 0, `[Normal]` for slot 1 and `[Difficult]` for slot 2. The section names describe the game from the side of the house that reads them. A computer house reading `[Easy]` is the opponent a player meets on Hard.

A scenario holds two difficulties, one for the player and one for the computer:

- In a campaign game, both come from [`Difficulty=`](/keys/difficulty/), the campaign setting kept in `sun.ini`. The player's difficulty is that setting, and the computer's is `2` minus it.
- A [launch file](/formats/spawn-ini/#a-campaign-mission) that starts a campaign mission sets the two separately instead, so it can pair any player difficulty with any computer difficulty.
- Outside a campaign, both come from the difficulty chosen for the session. The computer's is again `2` minus the player's.

With the menu's settings, the player's houses and the computer's houses read opposite sections:

| Setting chosen | A player-controlled house in a campaign | A computer house |
| --- | --- | --- |
| Easy | `[Easy]` | `[Difficult]` |
| Normal | `[Normal]` | `[Normal]` |
| Hard | `[Difficult]` | `[Easy]` |

Outside a campaign game, every human house plays at slot 1, whatever the session was set to. There, only the computer houses take their handicap from the setting.

A computer seat's `[HouseHandicaps]` entry in a [launch file](/formats/spawn-ini/#who-is-playing) sets that house's slot directly. The entry replaces the computer's slot from the table and [the multiplayer bonus](#the-computers-bonus-with-more-than-one-human).

A campaign mission names its difficulty in a message as it starts. The name follows the computer's difficulty and describes the game from the player's side. When the computer houses read `[Easy]` the mission is announced as Hard, `[Normal]` as Medium and `[Difficult]` as Easy. A launch file can supply its own name instead, so a client that offers more than three difficulties can name the one it chose.

In a campaign, each house gets its handicap as the game reads that house's section of the map. Only [`PlayerControl=yes`](/keys/playercontrol/) in that section puts the house in the player's column. More than one house may set it, and each one that does reads the player's section.

:::caution[Set `PlayerControl=yes` on a campaign's player house]
The map's `[Basic] Player=` entry does not put its house in the player's column. The game applies that entry only after every house already has its handicap. A player house without `PlayerControl=yes` in its section therefore plays the whole mission with the computer's handicap. At the Easy setting, that means `[Difficult]`. The next scenario assigns every house's handicap again.
:::

## What one difficulty section sets

A difficulty section has eight settings that take effect. The six multipliers do not all point the same way. A value above 1 makes the house deal more damage, move faster and take less damage. It also makes the house fire more slowly, pay more and build more slowly. Each house uses the values in its section as written; only the choice of section is inverted.

| Setting | Effect on a house in that slot |
| --- | --- |
| [`FirePower=`](/keys/firepower-difficulty-settings/) | Multiplies the damage of most weapons its objects fire; above 1 deals more. The key page lists the projectiles it does not scale. |
| [`Groundspeed=`](/keys/groundspeed/#scope-difficulty-settings) | Multiplies the speed of its ground movement; above 1 travels faster. |
| [`Armor=`](/keys/armor/#scope-difficulty-settings) | Divides the damage its objects take; above 1 takes less. |
| [`ROF=`](/keys/rof/#scope-difficulty-settings) | Multiplies the delay between its shots; above 1 fires more slowly. |
| [`Cost=`](/keys/cost/#scope-difficulty-settings) | Multiplies the prices it pays; above 1 pays more. |
| [`BuildTime=`](/keys/buildtime/#scope-difficulty-settings) | Multiplies the build time of everything it produces; above 1 builds more slowly. |
| [`RepairDelay=`](/keys/repairdelay/) | Sets how long a computer house waits after starting one building repair before it can start another. The wait is a random time between a quarter of the value and twice the value, in minutes. |
| [`DestroyWalls=`](/keys/destroywalls/) | `no` stops a computer house's objects from [scoring walls as targets](/systems/target-selection/#picking-the-winner). |

```ini title="rules.ini"
[Difficult] ; example values that weaken the house reading this section
FirePower=0.8
Groundspeed=0.9
Armor=0.8
ROF=1.2
Cost=1.2
BuildTime=1.2
RepairDelay=0.05
DestroyWalls=no
```

The linked key pages give the defaults.

The first seven settings are copied into the house when it gets its slot, as [the next section](#how-the-figures-are-combined) describes. `DestroyWalls` is read from the section whenever a computer object considers a wall. The section's current value therefore always applies, and no country setting changes it.

:::caution[Keep all three sections in the rules files]
Until some file supplies a difficulty section, every value in it is zero and each flag is `no`. A house in that slot deals no damage, has zero ground speed and pays nothing. Each hit it takes does only 1 point of damage, the least a hit can do. A computer house in that slot also ignores walls as targets, since `DestroyWalls` is `no`.

When no rules file supplies a section, the version a map or a campaign's companion file supplied stays in force for later scenarios in the same session, until another file supplies that section again. A rules file that supplies the section replaces the map's version at the next scenario load.
:::

:::caution[Repeat the whole section when overriding it]
A file that contains a difficulty section resets every key it leaves out to that key's built-in default. A map that declares `[Difficult]` to change one multiplier therefore returns the section's other keys to those defaults, not to the values in the rules files. Copy the whole section to keep the other values.
:::

Files that contain a difficulty section are read in this order: [each rules file in turn](/formats/rules-registries/), then the map. In a campaign, the companion `.INI` named for the scenario is read after every house already has its handicap. Its difficulty sections therefore change only `DestroyWalls`, not the seven figures the houses already hold.

## How the figures are combined

A house's seven copied figures are computed once, when the house gets its slot. They are not recomputed per shot, order or frame, so a later change to a difficulty section or a country reaches the house only when it is re-handicapped.

Outside a campaign game, six of the figures are also multiplied by the matching setting in the house's country section. A campaign game leaves the country out and keeps everything else:

| Difficulty setting | Campaign game | Outside a campaign |
| --- | --- | --- |
| `FirePower=` | On its own | Times the country's [`Firepower=`](/keys/firepower-housetype/) |
| `Groundspeed=` | Times [`GameSpeedBias`](/keys/gamespeedbias/) | Times the country's [`Groundspeed=`](/keys/groundspeed/#scope-housetype) and `GameSpeedBias` |
| `Armor=` | On its own | Times the country's [`Armor=`](/keys/armor/#scope-housetype) |
| `ROF=` | On its own | Times the country's [`ROF=`](/keys/rof/#scope-housetype) |
| `Cost=` | On its own | Times the country's [`Cost=`](/keys/cost/#scope-housetype) |
| `BuildTime=` | Times `GameSpeedBias` | Times the country's [`BuildTime=`](/keys/buildtime/#scope-housetype) and `GameSpeedBias` |
| `RepairDelay=` | Taken as written | Taken as written |

The country multipliers each default to 1, so a rules tree that never sets them gives the same figures in both columns.

[How long it takes](/systems/production/#how-long-it-takes) places the build-time figure in the production chain. [When the computer repairs](/systems/repair/#when-the-computer-repairs) covers how the repair delay is used.

## When a house is re-handicapped

A house is assigned its slot and its figures again at these points:

- **Campaign scenario load or restart.** Each house in the scenario's `[Houses]` list, as its section is read. Nothing carried over from the previous mission changes the slot.
- **Every other mode.** As the session's houses are assigned: human houses to slot 1, and computer houses to the computer's slot, adjusted by [the bonus below](#the-computers-bonus-with-more-than-one-human) or set by the launch file. The `Neutral` and `Special` houses created alongside them get no handicap, and all their multipliers stay at 1.
- **A player leaving a network game.** When [the computer takes over the departed player's base](/systems/leaving-a-match/#what-becomes-of-their-base), the house keeps slot 1 and keeps reading `[Normal]`, whatever the session was set to. It does not take the other computer houses' slot. Its figures are computed again and its team countdown restarts.

## The per-difficulty lists

`[General]` holds lists with one entry per difficulty slot. Each house reads the entry matching its slot, with no further inversion. For a computer house, entry 0 is therefore the one used at the Hard setting and entry 2 the one used at Easy.

These lists are read in every mode:

- [`TeamDelays`](/keys/teamdelays/), [`TotalAITeamCap`](/keys/totalaiteamcap/), [`MinimumAIDefensiveTeams`](/keys/minimumaidefensiveteams/), [`MaximumAIDefensiveTeams`](/keys/maximumaidefensiveteams/) and [`FillEarliestTeamProbability`](/keys/fillearliestteamprobability/).
- The twelve lists whose names begin `AIIonCannon`, one for each kind of target the computer's Ion Cannon rates, such as [`AIIonCannonEngineerValue`](/keys/aiioncannonengineervalue/).

[`AIHateDelays`](/keys/aihatedelays/) and [`MultiplayerAICM`](/keys/multiplayeraicm/) are applied once to each computer house, as a scenario outside a campaign finishes loading. A campaign game never uses them.

`MultiplayerAICM` gives a computer house extra money. Its entry is a percentage of the money the house holds at that moment, counting its credits and the value of its stored Tiberium, and that amount is added as credits. An entry of `100` leaves the house with twice its starting money, and `0` adds nothing.

The team-creation countdown that `TeamDelays` sets runs for every house, human or computer, and each house reads the entry for its slot. In a campaign, the player's house and the computer houses therefore read opposite ends of the list:

| Setting chosen | Entry a player-controlled campaign house reads | Entry a computer house reads |
| --- | --- | --- |
| Easy | 0 | 2 |
| Normal | 1 | 1 |
| Hard | 2 | 0 |

:::caution[The player's house reads `TeamDelays` from the hard end]
At the Easy setting, the player's house reads entry 0, the entry a computer house uses at the Hard setting. This matters when a map turns on [the AI-trigger switch](/systems/ai-team-production/#when-the-pass-runs) of the player's house: that house then draws AI triggers on entry 0's delay.
:::

[AI triggers and team production](/systems/ai-team-production/#difficulty) covers what the AI lists do. It also explains why an AI trigger's difficulty flags mean the same thing in both modes while these lists do not.

## The computer's bonus with more than one human

Outside a campaign game, [`CompEasyBonus=yes`](/keys/compeasybonus/) moves each computer house down one slot as it is assigned. It applies only when the session has more than one human entry, and a house already in slot 0 stays there. A skirmish set up from the menu has one human entry, so the bonus applies only in a LAN or online session.

Despite its name, the bonus makes the computer stronger. A computer house already holds the inverse of the player's setting, so moving it down a slot gives it the handicap of the next harder setting. The table follows one computer house through both steps. Its last column is never easier than the setting chosen.

| Setting chosen | Slot the computer house holds | Slot after the bonus | Setting it then behaves like |
| --- | --- | --- | --- |
| Easy | 2, the `[Difficult]` section | 1, the `[Normal]` section | Normal |
| Normal | 1, the `[Normal]` section | 0, the `[Easy]` section | Hard |
| Hard | 0, the `[Easy]` section | 0, the `[Easy]` section; no change | Hard |

## What else the slot decides

Each row below reads the house's slot directly. A row naming slot 0 or the `[Easy]` section describes a computer house when the player chose Hard. A row naming slot 2 describes one when the player chose Easy.

| Where | What the slot changes |
| --- | --- |
| [The computer's base plan](/systems/ai-base-building/#building-the-plan) | The `2 - slot` extra refineries and the `3 - slot` term in the extra defense placeholders |
| [The GDI wall ring](/systems/ai-base-building/#walls-and-gates) | The `3 - slot` term in the cap on wall defenses |
| A computer house's harvesters | It builds replacements until it owns one harvester per refinery in a campaign. Outside a campaign it builds up to two per refinery, or one in slot 2 |
| The computer's Ion Cannon | In slot 0 it also rates enemy objects still being built in a factory |
| Crushing an attacker | A computer house's vehicles in slot 2 never answer fire by driving over the attacker |

A map's triggers do not use a house's slot. They follow the scenario's difficulty, which is the player's difficulty in a campaign and the session's difficulty outside one. [Trigger springing](/systems/trigger-springing/#difficulty) covers which triggers that disables. AI triggers have a separate set of three difficulty flags, which decide whether the trigger can be drawn; [AI triggers and team production](/systems/ai-team-production/#difficulty) covers them.

## Parsed settings without effect

Four keys in the difficulty sections are read but change nothing:

- [`BuildSlowdown`](/keys/buildslowdown/) is never used.
- [`ContentScan`](/keys/contentscan/#scope-difficulty-settings) is tested only in code the game never runs.
- [`Airspeed`](/keys/airspeed/#scope-difficulty-settings) and [`BuildDelay`](/keys/builddelay/) are copied into the house's figures, but nothing uses those figures. A country's [`Airspeed=`](/keys/airspeed/#scope-housetype) is combined into the same unused figure.

Two related keys elsewhere have no effect either. [`FineDiffControl`](/keys/finediffcontrol/) in `[General]` is never used. The [`ContentScan`](/keys/contentscan/#scope-global-rules) threshold in `[IQ]` belongs to the same test as the difficulty section's `ContentScan`, which never runs.
